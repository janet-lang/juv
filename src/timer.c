#include "entry.h"

static int timer_mark(void *x, size_t s) {
    (void) s;
    uv_timer_t *t = (uv_timer_t *)x;
    if (NULL != t->data) {
        janet_mark(janet_wrap_fiber((JanetFiber *) t->data));
    }
    return 0;
}

static const JanetAbstractType timer_type = {
    "uv/timer",
    NULL,
    timer_mark,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static Janet cfun_timer_new(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    JanetFiber *fiber = janet_getfiber(argv, 0);
    uv_timer_t* handle = (uv_timer_t*) janet_abstract(&timer_type, sizeof(*handle));
    int ret = uv_timer_init(uv_default_loop(), handle);
    if (ret < 0) janet_panic("could not create timer");
    Janet val = janet_wrap_abstract(handle);
    janet_gcroot(val);
    handle->data = fiber;
    return val;
}

static void timer_cleanup(uv_timer_t *t) {
    int res = uv_timer_stop(t);
    janet_gcunroot(janet_wrap_abstract(t));
    t->data = NULL;
    if (res < 0) juv_panic(res);
}

static void juv_timer_cb(uv_timer_t* handle) {
    JanetFiber *fiber = (JanetFiber *)(handle->data);
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
    uv_timer_t *handle = janet_getabstract(argv, 0, &timer_type);
    if (handle->data == NULL) {
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
    uv_timer_t *handle = janet_getabstract(argv, 0, &timer_type);
    if (handle->data == NULL) return janet_wrap_nil();
    timer_cleanup(handle);
    return janet_wrap_nil();
}

static Janet cfun_timer_again(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    uv_timer_t *handle = janet_getabstract(argv, 0, &timer_type);
    if (handle->data == NULL) return janet_wrap_nil();
    int ret = uv_timer_again(handle);
    if (ret < 0) juv_panic(ret);
    return argv[0];
}

static Janet cfun_timer_repeat(int32_t argc, Janet *argv) {
    janet_arity(argc, 1, 2);
    uv_timer_t *handle = janet_getabstract(argv, 0, &timer_type);
    if (handle->data == NULL) return janet_wrap_nil();
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
