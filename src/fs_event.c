#include "entry.h"

static int fs_event_method_get(void *p, Janet key, Janet *out);
static const JanetAbstractType fs_event_type = {
    "uv/fs-event",
    NULL,
    juv_handle_mark,
    fs_event_method_get,
    JANET_ATEND_GET
};

static Janet cfun_fs_event_new(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    JanetFiber *fiber = janet_getfiber(argv, 0);
    uv_fs_event_t* handle = (uv_fs_event_t*) juv_makehandle(&fs_event_type, sizeof(*handle));
    int ret = uv_fs_event_init(uv_default_loop(), handle);
    if (ret < 0) janet_panic("could not create fs event monitor");
    Janet val = juv_wrap_handle(handle);
    janet_gcroot(val);
    juv_handle_setfiber(handle, 0, fiber);
    return val;
}

static void fs_event_cleanup(uv_fs_event_t *t) {
    int res = uv_fs_event_stop(t);
    janet_gcunroot(juv_wrap_handle(t));
    juv_handle_setfiber(t, 0, NULL);
    juv_handle_setcb(t, 1, NULL);
    if (res < 0) juv_panic(res);
}

static void fs_event_cb(uv_fs_event_t *handle, const char *filename, int events, int status) {
    if (status) {
        juv_panic(status);
    } else {
        JanetFunction *cb = juv_handle_cb(handle, 1);
        Janet argv[] = { juv_wrap_handle(handle),
                         janet_wrap_string(janet_cstring(filename)),
                         janet_wrap_integer(events) };
        Janet out = janet_wrap_nil();
        JanetFiber *cb_f = janet_fiber(cb, 64, 3, argv);
        JanetSignal sig;
        if (!cb_f) {
            out = janet_cstringv("arity mismatch");
            sig = JANET_SIGNAL_ERROR;
        } else {
            JanetFiber *handle_f = juv_handle_fiber(handle, 0);
            cb_f->env = handle_f->env;
            sig = janet_continue(cb_f, janet_wrap_nil(), &out);
        }
        if (sig != JANET_SIGNAL_OK && sig != JANET_SIGNAL_YIELD) {
            janet_stacktrace(cb_f, out);
            juv_toperror(sig, out);
        }
    }
}

static Janet cfun_fs_event_start(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 4);
    uv_fs_event_t *handle = juv_gethandle(argv, 0, &fs_event_type);
    JanetFunction *cb = janet_getfunction(argv, 1);
    if (NULL != juv_handle_cb(handle, 1)) {
        janet_panic("cannot have multiple callbacks on fs event monitor");
    }
    juv_handle_setcb(handle, 1, cb);
    const char *path = janet_getcstring(argv, 2);
    uint64_t flags = janet_getinteger(argv, 3);
    int ret = uv_fs_event_start(handle, fs_event_cb, path, flags);
    if (ret < 0) juv_panic(ret);
    return argv[0];
}

static Janet cfun_fs_event_stop(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    uv_fs_event_t *handle = juv_gethandle(argv, 0, &fs_event_type);
    if (juv_handle_fiber(handle, 1) == NULL) return janet_wrap_nil();
    fs_event_cleanup(handle);
    return janet_wrap_nil();
}

static Janet cfun_fs_event_getpath(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    uv_fs_event_t *handle = juv_gethandle(argv, 0, &fs_event_type);
    char buf[1024];
    size_t pathlen = 1023;
    int r = uv_fs_event_getpath(handle, (char *)&buf, &pathlen);
    if (r) juv_panic(r);
    JanetString pathname = janet_string((uint8_t *)buf, pathlen);
    return janet_wrap_string(pathname);
}

static const JanetMethod fs_event_methods[] = {
    {"start", cfun_fs_event_start},
    {"stop", cfun_fs_event_stop},
    {"getpath", cfun_fs_event_getpath},
    {NULL, NULL}
};

static int fs_event_method_get(void *p, Janet key, Janet *out) {
    (void) p;
    if (!janet_checktype(key, JANET_KEYWORD))
        janet_panicf("expected keyword, got %v", key);
    return janet_getmethod(janet_unwrap_keyword(key), fs_event_methods, out);
}

static const JanetReg cfuns[] = {
    {"fs-event/new", cfun_fs_event_new, NULL},
    {"fs-event/start", cfun_fs_event_start, NULL},
    {"fs-event/stop", cfun_fs_event_stop, NULL},
    {"fs-event/getpath", cfun_fs_event_getpath, NULL},
    {NULL, NULL, NULL}
};

void submod_fs_event(JanetTable *env) {
    janet_cfuns(env, "uv", cfuns);
}
