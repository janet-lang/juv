#include "handle.h"
#include <stddef.h>

void *juv_abstract2handle(void *x) {
    JuvHandle *abst = (JuvHandle *)x;
    return (void *) &(abst->aligner[0]);
}

void *juv_handle2abstract(void *h) {
    return (char *)(h) - offsetof(JuvHandle, aligner);
}

void *juv_makehandle(const JanetAbstractType *atype, size_t size) {
    JuvHandle *raw = (JuvHandle *) janet_abstract(atype, size + sizeof(JuvHandle));
    raw->cbdata[0] = janet_wrap_nil();
    raw->cbdata[1] = janet_wrap_nil();
    void *abst = juv_abstract2handle(raw);
    ((uv_handle_t *)abst)->data = NULL;
    return abst;
}

void *juv_gethandle(const Janet *argv, int32_t n, const JanetAbstractType *atype) {
    JuvHandle *raw = janet_getabstract(argv, n, atype);
    return juv_abstract2handle(raw);
}

Janet juv_wrap_handle(void *handle) {
    return janet_wrap_abstract(juv_handle2abstract(handle));
}

JanetFiber *juv_handle_fiber(void *handle, int index) {
    JuvHandle *abst = juv_handle2abstract(handle);
    Janet x = abst->cbdata[index];
    return (janet_checktype(x, JANET_FIBER))
        ? janet_unwrap_fiber(x)
        : NULL;
}

void juv_handle_setfiber(void *handle, int index, JanetFiber *fiber) {
    JuvHandle *abst = juv_handle2abstract(handle);
    abst->cbdata[index] = fiber ? janet_wrap_fiber(fiber) : janet_wrap_nil();
}

JanetFunction *juv_handle_cb(void *handle, int index) {
    JuvHandle *abst = juv_handle2abstract(handle);
    Janet x = abst->cbdata[index];
    return (janet_checktype(x, JANET_FUNCTION))
        ? janet_unwrap_function(x)
        : NULL;
}

void juv_handle_setcb(void *handle, int index, JanetFunction *cb) {
    JuvHandle *abst = juv_handle2abstract(handle);
    abst->cbdata[index] = cb ? janet_wrap_function(cb) : janet_wrap_nil();
}

void juv_handle_checkfree(void *handle, int index) {
    JuvHandle *abst = juv_handle2abstract(handle);
    if (!janet_checktype(abst->cbdata[index], JANET_NIL)) {
        janet_panicf("cannot set callback or fiber on %v, already set", janet_wrap_abstract(abst));
    }
}

int juv_handle_mark(void *x, size_t s) {
    (void) s;
    JuvHandle *h = (JuvHandle *)x;
    for (int i = 0; i < 2; i++)
        janet_mark(h->cbdata[i]);
    return 0;
}

