#include "entry.h"

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

void juv_schedule(uv_handle_t *req) {
    JanetFiber *fiber = janet_current_fiber();
    janet_gcroot(janet_wrap_fiber(fiber));
    req->data = fiber;
}

void juv_toperror(JanetSignal sig, Janet out) {
    /* Top level error, eventually should register some kind
     * of handler */
    janet_printf("uv top level signal(%d): %v\n", sig, out);
}

void juv_resume(uv_handle_t *req, Janet value, int freemem) {
    JanetFiber *fiber = (JanetFiber *)(req->data);
    if (freemem) free(req);
    janet_gcunroot(janet_wrap_fiber(fiber));
    Janet out;
    JanetSignal sig = janet_continue(fiber, value, &out);
    if (sig == JANET_SIGNAL_YIELD || sig == JANET_SIGNAL_OK) {
        ;
    } else {
        juv_toperror(sig, out);
    }
}

