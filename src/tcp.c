#include "entry.h"
#include "stream.h"

static Janet tcp_method_get(void *p, Janet key);
const JanetAbstractType tcp_type = {
    "uv/tcp",
    NULL,
    juv_handle_mark,
    tcp_method_get,
    NULL,
    NULL,
    NULL,
    NULL
};

static Janet juv_tcp_new(int32_t argc, Janet *argv) {
    janet_arity(argc, 0, 1);
    uv_tcp_t *tcp = (uv_tcp_t*) juv_makehandle(&tcp_type, sizeof(*tcp));

    /* TODO - use flags */
    uint64_t flags = 0;
    if (argc) flags = janet_getflags(argv, 0, "");
    (void) flags;

    /* Intialize */
    int ret = uv_tcp_init(uv_default_loop(), tcp);
    if (ret) janet_panic("could not create tcp socket");

    Janet val = juv_wrap_handle(tcp);
    janet_gcroot(val);
    return val;
}

static Janet juv_tcp_bind(int32_t argc, Janet *argv) {
    janet_arity(argc, 3, 4);
    uv_tcp_t *tcp = juv_gethandle(argv, 0, &tcp_type);

    /* TODO - use flags */
    uint64_t flags = 0;
    if (argc > 3) flags = janet_getflags(argv, 3, "");
    (void) flags;

    /* Get socket address */
    struct sockaddr_storage *addr = juv_malloc(sizeof(struct sockaddr_storage));
    juv_get_ipaddr(argv, 1, addr);

    /* Bind */
    int r = uv_tcp_bind(tcp, (const struct sockaddr *)addr, 0);
    if (r) juv_panic(r);

    return argv[0];
}

static void cb_connect(uv_connect_t *req, int status) {
    if (status) {
        juv_panic(status);
    } else {
        juv_resume_req((uv_req_t *)req, juv_wrap_handle(req->handle));
    }
}
static Janet juv_tcp_connect(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 3);
    uv_tcp_t *tcp = juv_gethandle(argv, 0, &tcp_type);

    /* Get socket address */
    struct sockaddr_storage addr;
    juv_get_ipaddr(argv, 1, &addr);

    /* Connect */
    uv_connect_t *conn = juv_malloc(sizeof(uv_connect_t));
    int r = uv_tcp_connect(conn, tcp, (const struct sockaddr*)&addr, cb_connect);
    if (r) juv_panic(r);
    juv_schedule_req((uv_req_t *)conn);
    janet_gcroot(argv[0]);

    return argv[0];
}

static Janet make_name(struct sockaddr_storage *addr) {
    char buf[64] = {'\0'};
    Janet *tup = janet_tuple_begin(2);
    if (addr->ss_family == AF_INET) {
        struct sockaddr_in *v4 = (struct sockaddr_in *)addr;
        int r = uv_ip4_name(v4, (char *)buf, sizeof(buf) - 1);
        if (r) juv_panic(r);
        tup[1] = janet_wrap_integer(v4->sin_port);
    } else {
        struct sockaddr_in6 *v6 = (struct sockaddr_in6 *)addr;
        int r = uv_ip6_name(v6, (char *)buf, sizeof(buf) - 1);
        if (r) juv_panic(r);
        tup[1] = janet_wrap_integer(v6->sin6_port);
    }
    tup[0] = janet_cstringv(buf);
    return janet_wrap_tuple(janet_tuple_end(tup));
}

static Janet juv_tcp_sockname(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    uv_tcp_t *tcp = juv_gethandle(argv, 0, &tcp_type);
    struct sockaddr_storage buf;
    int namelen = 0;
    int r = uv_tcp_getsockname(tcp, (struct sockaddr *)&buf, &namelen);
    if (r) juv_panic(r);
    return make_name(&buf);
}

static Janet juv_tcp_peername(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    uv_tcp_t *tcp = juv_gethandle(argv, 0, &tcp_type);
    struct sockaddr_storage buf;
    int namelen = 0;
    int r = uv_tcp_getpeername(tcp, (struct sockaddr *)&buf, &namelen);
    if (r) juv_panic(r);
    return make_name(&buf);
}

static const JanetReg cfuns[] = {
    {"tcp/new", juv_tcp_new, NULL},
    {"tcp/bind", juv_tcp_bind, NULL},
    {"tcp/connect", juv_tcp_connect, NULL},
    {"tcp/sockname", juv_tcp_sockname, NULL},
    {"tcp/peername", juv_tcp_peername, NULL},
    {NULL, NULL, NULL}
};

static const JanetMethod tcp_methods[] = {
    /* Stream methods */
    {"shutdown", juv_shutdown},
    {"listen", juv_listen},
    {"accept", juv_accept},
    {"read-start", juv_read_start},
    {"read-stop", juv_read_stop},
    {"write", juv_write},
    {"try-write", juv_try_write},
    {"readable?", juv_is_readable},
    {"writeable?", juv_is_writeable},
    {"write-queue-size", juv_stream_get_write_queue_size},
    /* TCP methods */
    {"new", juv_tcp_new},
    {"bind", juv_tcp_bind},
    {"connect", juv_tcp_connect},
    {"sockname", juv_tcp_sockname},
    {"peername", juv_tcp_peername},
    {NULL, NULL}
};

static Janet tcp_method_get(void *p, Janet key) {
    (void) p;
    if (!janet_checktype(key, JANET_KEYWORD))
        janet_panicf("expected keyword, got %v", key);
    return janet_getmethod(janet_unwrap_keyword(key), tcp_methods);
}

void submod_tcp(JanetTable *env) {
    janet_cfuns(env, "uv", cfuns);
}
