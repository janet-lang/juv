#include "entry.h"
#include <stddef.h>

int juv_parse_flags(const uint8_t *arg, const char *chars, const int *flags) {
    int ret = 0;
    for (const uint8_t *c = arg; *c; c++) {
        int mask = 0;
        for (const char *test = chars; *test; test++) {
            if ((uint8_t) *test == *c) {
                mask = flags[test - chars];
                break;
            }
        }
        ret |= mask;
    }
    return ret;
}

void juv_panic(int r) {
    janet_panic(uv_strerror(r));
}

void juv_schedule(uv_handle_t *req, int index) {
    JanetFiber *fiber = janet_current_fiber();
    janet_gcroot(janet_wrap_fiber(fiber));
    juv_handle_setfiber(req, index, fiber);
}

void juv_schedule_req(uv_req_t *req) {
    JanetFiber *fiber = janet_current_fiber();
    janet_gcroot(janet_wrap_fiber(fiber));
    req->data = fiber;
}

void juv_toperror(JanetSignal sig, Janet out) {
    /* Top level error, eventually should register some kind
     * of handler */
    janet_printf("uv top level signal(%d): %v\n", sig, out);
}

void juv_resume(uv_handle_t *handle, int index, Janet value) {
    JanetFiber *fiber = juv_handle_fiber(handle, index);
    if (NULL == fiber) return;
    janet_gcunroot(janet_wrap_fiber(fiber));
    Janet out;
    JanetSignal sig = janet_continue(fiber, value, &out);
    if (sig == JANET_SIGNAL_YIELD || sig == JANET_SIGNAL_OK) {
        ;
    } else {
        juv_toperror(sig, out);
    }
}

void juv_resume_req(uv_req_t *req, Janet value) {
    JanetFiber *fiber = (JanetFiber *)(req->data);
    if (NULL == fiber) return;
    free(req);
    janet_gcunroot(janet_wrap_fiber(fiber));
    Janet out;
    JanetSignal sig = janet_continue(fiber, value, &out);
    if (sig == JANET_SIGNAL_YIELD || sig == JANET_SIGNAL_OK) {
        ;
    } else {
        juv_toperror(sig, out);
    }
}

void *juv_malloc(size_t x) {
    void *ret = malloc(x);
    if (NULL == ret) {
        janet_panicf("out of memory");
    }
    return ret;
}

void juv_get_ipaddr(const Janet *argv, int32_t n, struct sockaddr_storage *addr) {
    const char *cstr = janet_getcstring(argv, n);
    int32_t port = janet_getinteger(argv, n + 1);
    int res = uv_ip4_addr(cstr, port, (struct sockaddr_in *)addr);
    if (res) {
        res = uv_ip6_addr(cstr, port, (struct sockaddr_in6 *)addr);
        if (res) {
            uv_getaddrinfo_t req;
            res = uv_getaddrinfo(uv_default_loop(),
                    &req,
                    NULL,
                    cstr,
                    NULL,
                    NULL);
            if (res) {
                juv_panic(res);
            } else {
                char address_string[65] = {'\0'};
                if (req.addrinfo->ai_family == AF_INET) {
                    /* Ip v4 */
                    uv_ip4_name((struct sockaddr_in*) req.addrinfo->ai_addr, address_string, 16);
                    res = uv_ip4_addr(address_string, port, (struct sockaddr_in *) addr);
                } else {
                    /* Ip v6 */
                    uv_ip6_name((struct sockaddr_in6*) req.addrinfo->ai_addr, address_string, 64);
                    res = uv_ip6_addr(address_string, port, (struct sockaddr_in6 *) addr);
                }
                if (res) {
                    janet_panicf("could not get ip address from %v", argv[n]);
                }
                uv_freeaddrinfo(req.addrinfo);
            }
        }
    }
}
