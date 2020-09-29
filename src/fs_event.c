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
    juv_handle_setfiber(handle, 1, fiber);
    return val;
}

static void fs_event_cleanup(uv_fs_event_t *t) {
    int res = uv_fs_event_stop(t);
    janet_gcunroot(juv_wrap_handle(t));
    juv_handle_setfiber(t, 1, NULL);
    if (res < 0) juv_panic(res);
}

static void juv_fs_event_cb(uv_fs_event_t* handle, const char *filename, int events, int status) {
    JanetFiber *fiber = juv_handle_fiber(handle, 1);
    if (fiber == NULL) {
        fs_event_cleanup(handle);
        return;
    }
    Janet out;
    JanetSignal sig = janet_continue(fiber, janet_wrap_nil(), &out);
    if (sig == JANET_SIGNAL_YIELD) {
        ;
    } else if (sig == JANET_SIGNAL_OK) {
        fs_event_cleanup(handle);
    } else {
        fs_event_cleanup(handle);
        juv_toperror(sig, out);
    }
}

static Janet cfun_fs_event_start(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 3);
    uv_fs_event_t *handle = juv_gethandle(argv, 0, &fs_event_type);
    if (juv_handle_fiber(handle, 1) == NULL) {
        janet_panic("fs event monitor has been destroyed, cannot be restarted");
    }
    const char *path = janet_getcstring(argv, 1);
    uint64_t flags = janet_getinteger(argv, 2);
    int ret = uv_fs_event_start(handle, juv_fs_event_cb, path, flags);
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

static const JanetMethod fs_event_methods[] = {
    {"start", cfun_fs_event_start},
    {"stop", cfun_fs_event_stop},
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
    {NULL, NULL, NULL}
};

void submod_fs_event(JanetTable *env) {
    janet_cfuns(env, "uv", cfuns);
}
