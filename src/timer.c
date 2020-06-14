#include "entry.h"

static int timer_method_get(void *p, Janet key, Janet *out);
static const JanetAbstractType timer_type = {
    "uv/timer",
    NULL,
    juv_handle_mark,
    timer_method_get,
    JANET_ATEND_GET
};

static Janet cfun_timer_new(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    JanetFiber *fiber = janet_getfiber(argv, 0);
    uv_timer_t* handle = (uv_timer_t*) juv_makehandle(&timer_type, sizeof(*handle));
    int ret = uv_timer_init(uv_default_loop(), handle);
    if (ret < 0) janet_panic("could not create timer");
    Janet val = juv_wrap_handle(handle);
    janet_gcroot(val);
    juv_handle_setfiber(handle, 1, fiber);
    return val;
}

static void timer_cleanup(uv_timer_t *t) {
    int res = uv_timer_stop(t);
    janet_gcunroot(juv_wrap_handle(t));
    juv_handle_setfiber(t, 1, NULL);
    if (res < 0) juv_panic(res);
}

static void juv_timer_cb(uv_timer_t* handle) {
    JanetFiber *fiber = juv_handle_fiber(handle, 1);
    if (fiber == NULL) {
        timer_cleanup(handle);
        return;
    }
    Janet out;
    JanetSignal sig = janet_continue(fiber, janet_wrap_nil(), &out);
    if (sig == JANET_SIGNAL_YIELD) {
        ;
    } else if (sig == JANET_SIGNAL_OK) {
        timer_cleanup(handle);
    } else {
        timer_cleanup(handle);
        juv_toperror(sig, out);
    }
}

static Janet cfun_timer_start(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 3);
    uv_timer_t *handle = juv_gethandle(argv, 0, &timer_type);
    if (juv_handle_fiber(handle, 1) == NULL) {
        janet_panic("timer has been destroyed, cannot be restarted");
    }
    uint64_t repeat = janet_getinteger(argv, 1);
    uint64_t timeout = janet_getinteger(argv, 2);
    int ret = uv_timer_start(handle, juv_timer_cb, timeout, repeat);
    if (ret < 0) juv_panic(ret);
    return argv[0];
}

static Janet cfun_timer_stop(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    uv_timer_t *handle = juv_gethandle(argv, 0, &timer_type);
    if (juv_handle_fiber(handle, 1) == NULL) return janet_wrap_nil();
    timer_cleanup(handle);
    return janet_wrap_nil();
}

static Janet cfun_timer_again(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    uv_timer_t *handle = juv_gethandle(argv, 0, &timer_type);
    if (juv_handle_fiber(handle, 1) == NULL) return janet_wrap_nil();
    int ret = uv_timer_again(handle);
    if (ret < 0) juv_panic(ret);
    return argv[0];
}

static Janet cfun_timer_repeat(int32_t argc, Janet *argv) {
    janet_arity(argc, 1, 2);
    uv_timer_t *handle = juv_gethandle(argv, 0, &timer_type);
    if (juv_handle_fiber(handle, 1) == NULL) return janet_wrap_nil();
    if (argc == 1) {
        /* get */
        uint64_t repeat = uv_timer_get_repeat(handle);
        return janet_wrap_number(repeat);
    } else {
        /* set */
        uint64_t repeat = janet_getinteger(argv, 1);
        uv_timer_set_repeat(handle, repeat);
        return argv[0];
    }
}
 
static const JanetMethod timer_methods[] = {
    {"start", cfun_timer_start},
    {"stop", cfun_timer_stop},
    {"again", cfun_timer_again},
    {"repeat", cfun_timer_repeat},
    {NULL, NULL}
};

static int timer_method_get(void *p, Janet key, Janet *out) {
    (void) p;
    if (!janet_checktype(key, JANET_KEYWORD))
        janet_panicf("expected keyword, got %v", key);
    return janet_getmethod(janet_unwrap_keyword(key), timer_methods, out);
}

static const JanetReg cfuns[] = {
    {"timer/new", cfun_timer_new, NULL},
    {"timer/start", cfun_timer_start, NULL},
    {"timer/stop", cfun_timer_stop, NULL},
    {"timer/again", cfun_timer_again, NULL},
    {"timer/repeat", cfun_timer_repeat, NULL},
    {NULL, NULL, NULL}
};

void submod_timer(JanetTable *env) {
    janet_cfuns(env, "uv", cfuns);
}
