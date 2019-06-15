#ifndef HANDLE_H_PTDMI5JA
#define HANDLE_H_PTDMI5JA

#include <uv.h>
#include <janet.h>

/* Map event types to an index into the cbdata array. */
#define JUV_READ 1
#define JUV_LISTEN 1

typedef struct JuvHandle {
    Janet cbdata[2];
    long long aligner[];
} JuvHandle;

/* Abstraction around all libuv handle types. Makes it easier
 * to pass around fibers, and also free up uv_handle_t->data
 * for other uses / easier interop. */
void *juv_abstract2handle(void *abst);
void *juv_handle2abstract(void *h);
void *juv_makehandle(const JanetAbstractType *atype, size_t size);
void *juv_gethandle(const Janet *argv, int32_t n, const JanetAbstractType *atype);
Janet juv_wrap_handle(void *handle);
int juv_handle_mark(void *x, size_t s);

/* Handle getting and setting callbacks and fibers.
 * Caller is responsible for knowing if the handle
 * set a callback function or fiber. */
JanetFiber *juv_handle_fiber(void *handle, int index);
void juv_handle_setfiber(void *handle, int index, JanetFiber *fiber);
JanetFunction *juv_handle_cb(void *handle, int index);
void juv_handle_setcb(void *handle, int index, JanetFunction *cb);
void juv_handle_checkfree(void *handle, int index);

#endif /* end of include guard: HANDLE_H_PTDMI5JA */
