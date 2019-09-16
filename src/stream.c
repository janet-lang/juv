#include "stream.h"
#include "handle.h"

/* Most of the functionality here is enabling methods
 * used by other concrete classes */

uv_stream_t *juv_getstream(const Janet *argv, int32_t n) {
    Janet x = argv[n];
    if (!janet_checktype(x, JANET_ABSTRACT)) goto bad;
    void *abstractx = janet_unwrap_abstract(x);
    const JanetAbstractType *tp = janet_abstract_type(abstractx); 
    if (tp == &tcp_type ||
            tp == &pipe_type ||
            tp == &tty_type) {
        return juv_abstract2handle(abstractx);
    }
bad:
    janet_panicf("bad slot #%d: expected uv stream type, got %v", n, x);
}

void cb_shutdown(uv_shutdown_t* req, int status) {
    if (status) {
        juv_panic(status);
    } else {
        juv_resume_req((uv_req_t *)req, juv_wrap_handle(req->handle));
    }
}
Janet juv_shutdown(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    uv_stream_t *stream = juv_getstream(argv, 0);
    uv_shutdown_t *req = juv_malloc(sizeof(uv_shutdown_t));
    int r = uv_shutdown(req, stream, cb_shutdown);
    if (r) juv_panic(r);
    juv_schedule_req((uv_req_t *) req);
    return argv[0];
}

void cb_listen(uv_stream_t* server, int status) {
    if (status) {
        juv_panic(status);
    } else {
        JanetFunction *cb = juv_handle_cb(server, JUV_LISTEN);
        Janet argv[] = { juv_wrap_handle(server) };
        Janet out = janet_wrap_nil();
        JanetFiber *f = NULL;
        JanetSignal sig = janet_pcall(cb, 1, argv, &out, &f);
        if (sig != JANET_SIGNAL_OK && sig != JANET_SIGNAL_YIELD)
            juv_toperror(sig, out);
    }
}
Janet juv_listen(int32_t argc, Janet *argv) {
    janet_arity(argc, 2, 3);
    uv_stream_t *stream = juv_getstream(argv, 0);
    JanetFunction *cb = janet_getfunction(argv, 1);
    int32_t backlog = (argc > 2) ? janet_getinteger(argv, 2) : 128;
    if (NULL != juv_handle_cb(stream, JUV_LISTEN)) {
        janet_panic("cannot have multiple listeners on one stream");
    }
    juv_handle_setcb(stream, JUV_LISTEN, cb);
    int r = uv_listen(stream, backlog, cb_listen);
    if (r) juv_panic(r);
    janet_gcroot(argv[0]);
    return argv[0];
}

Janet juv_accept(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 2);
    uv_stream_t *server = juv_getstream(argv, 0);
    uv_stream_t *client = juv_getstream(argv, 1);
    int r = uv_accept(server, client);
    if (r) juv_panic(r);
    return argv[1];
}

static void cb_read_start_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    (void) handle;
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
}
static void cb_read_start_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    Janet val;
    if (nread <= 0) {
        val = janet_wrap_nil();
    } else {
        JanetBuffer *buffer = janet_buffer(0);
        janet_buffer_push_bytes(buffer, (uint8_t *) buf->base, nread);
        free(buf->base);
        val = janet_wrap_buffer(buffer);
    }
    juv_resume((uv_handle_t *)stream, JUV_READ, val);
}
Janet juv_read_start(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    uv_stream_t *stream = juv_getstream(argv, 0);
    juv_handle_checkfree(stream, JUV_READ);
    int r = uv_read_start(stream, cb_read_start_alloc, cb_read_start_read);
    if (r) juv_panic(r);
    juv_schedule((uv_handle_t *) stream, JUV_READ);
    janet_gcroot(argv[0]);
    return janet_wrap_nil();
}

Janet juv_read_stop(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    uv_stream_t *stream = juv_getstream(argv, 0);
    int r = uv_read_stop(stream);
    if (r) juv_panic(r);
    juv_handle_setfiber(stream, JUV_READ, NULL);
    return janet_wrap_nil();
}

typedef struct {
    uv_write_t wrt;
    int is_buffer;
    uv_buf_t bufs[1];
} JuvWrite;
static void cb_write(uv_write_t *req, int status) {
    if (status) {
        printf("write failed: (%d)\n", status);
    } else {
        JuvWrite *jw = (JuvWrite *)req;
        if (jw->is_buffer) {
            free(jw->bufs[0].base);
        } else {
            janet_gcunroot(janet_wrap_string(jw->bufs[0].base));
        }
        juv_resume_req((uv_req_t *) req, janet_wrap_nil());
    }
}
Janet juv_write(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 2);
    uv_stream_t *stream = juv_getstream(argv, 0);

    /* Handle buffers differently from immutable byte
     * sequences. */
    JuvWrite *jw = juv_malloc(sizeof(JuvWrite));
    if (janet_checktype(argv[1], JANET_BUFFER)) {
        jw->is_buffer = 1;
        JanetBuffer *buf = janet_unwrap_buffer(argv[1]);
        jw->bufs[0].base = (char *) juv_malloc(buf->count);
        memcpy(jw->bufs[0].base, buf->data, buf->count);
        jw->bufs[0].len = buf->count;
    } else {
        jw->is_buffer = 0;
        JanetByteView bv = janet_getbytes(argv, 1);
        /* Is the constness an issue? */
        jw->bufs[0].base = (char *) bv.bytes;
        jw->bufs[0].len = bv.len;
        janet_gcroot(argv[1]);
    }

    int r = uv_write(&(jw->wrt), stream, &(jw->bufs[0]), 1, cb_write);
    if (r < 0) return janet_wrap_nil();
    juv_schedule_req((uv_req_t *) jw);
    return janet_wrap_integer(r);
}

Janet juv_try_write(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 2);
    uv_stream_t *stream = juv_getstream(argv, 0);
    JanetByteView bytes = janet_getbytes(argv, 1);
    uv_buf_t bufs[1];
    bufs[0].base = (char *) bytes.bytes;
    bufs[0].len = bytes.len;
    int r = uv_try_write(stream, bufs, 1);
    if (r < 0) return janet_wrap_nil();
    return janet_wrap_integer(r);
}

Janet juv_is_readable(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    uv_stream_t *stream = juv_getstream(argv, 0);
    return janet_wrap_boolean(uv_is_readable(stream));
}

Janet juv_is_writeable(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    uv_stream_t *stream = juv_getstream(argv, 0);
    return janet_wrap_boolean(uv_is_writable(stream));
}

Janet juv_stream_get_write_queue_size(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    uv_stream_t *stream = juv_getstream(argv, 0);
    return janet_wrap_number((double) stream->write_queue_size);
}

static const JanetReg cfuns[] = {
    {"shutdown", juv_shutdown, NULL},
    {"listen", juv_listen, NULL},
    {"accept", juv_accept, NULL},
    {"read-start", juv_read_start, NULL},
    {"read-stop", juv_read_stop, NULL},
    {"write", juv_write, NULL},
    {"try-write", juv_try_write, NULL},
    {"readable?", juv_is_readable, NULL},
    {"writeable?", juv_is_writeable, NULL},
    {"write-queue-size", juv_stream_get_write_queue_size, NULL},
    {NULL, NULL, NULL}
};

void submod_stream(JanetTable *env) {
    janet_cfuns(env, "uv", cfuns);
}
