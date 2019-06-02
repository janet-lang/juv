#include "entry.h"

static int get_file_flags(const Janet *argv, int32_t n) {
    static const int flags[] = {
        UV_FS_O_APPEND,
        UV_FS_O_CREAT,
        UV_FS_O_DIRECT,
        UV_FS_O_DIRECTORY,
        UV_FS_O_DSYNC,
        UV_FS_O_EXCL,
        UV_FS_O_EXLOCK,
        UV_FS_O_NOATIME,
        UV_FS_O_NOCTTY,
        UV_FS_O_NOFOLLOW,
        UV_FS_O_NONBLOCK,
        UV_FS_O_RANDOM,
        UV_FS_O_RDONLY,
        UV_FS_O_RDWR,
        UV_FS_O_SEQUENTIAL,
        UV_FS_O_SHORT_LIVED,
        UV_FS_O_SYMLINK,
        UV_FS_O_SYNC,
        UV_FS_O_TEMPORARY,
        UV_FS_O_TRUNC,
        UV_FS_O_WRONLY
    };
    static const char chars[] = "acdDyElACFBRr+sLSntTw";
    const uint8_t *mode = janet_getkeyword(argv, n);
    return juv_parse_flags(mode, chars, flags);
}

static const JanetAbstractType file_type = {
    "uv/file",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

/* Wrap a file descriptor */
static Janet wrap_file(int file) {
    int *abst = janet_abstract(&file_type, sizeof(int));
    *abst = file;
    return janet_wrap_abstract(abst);
}

/* Unwrap a file descriptor */
static int getfile(const Janet *argv, int32_t n) {
    void *abst = janet_getabstract(argv, n, &file_type);
    return *((int *)abst);
}

/* UV open */
static void cb_open(uv_fs_t *req) {
    int fd = req->result;
    juv_last_error = fd;
    uv_fs_req_cleanup(req);
    juv_resume((uv_handle_t *)req, wrap_file(fd), 1);
}
static Janet cfun_fs_open(int32_t argc, Janet *argv) {
    janet_arity(argc, 1, 3);
    const char *path = janet_getcstring(argv, 0);
    int flags = 0;
    if (argc >= 2) {
        flags = get_file_flags(argv, 1);
    }
    int mode = 0;
    if (argc >= 3) {
        mode = janet_getinteger(argv, 2);
    }
    uv_fs_t *openreq = malloc(sizeof(uv_fs_t));
    if (!openreq) janet_panic("no mem.");
    int r = uv_fs_open(uv_default_loop(), openreq, path, flags, mode, cb_open);
    if (r < 0) juv_panic(r);

    juv_schedule((uv_handle_t *) openreq);

    return janet_wrap_nil();
}

/* UV close */
static void cb_close(uv_fs_t *req) {
    uv_fs_req_cleanup(req);
    juv_resume((uv_handle_t *)req, janet_wrap_nil(), 1);
}
static Janet cfun_fs_close(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 1);
    int fd = getfile(argv, 0);
    uv_fs_t *closereq = malloc(sizeof(uv_fs_t));
    uv_fs_close(uv_default_loop(), closereq, fd, cb_close);
    return janet_wrap_nil();
}

/* UV read */
struct juv_read_state {
    uv_fs_t req;
    uv_buf_t bufs[1];
    char mem[];
};
static void cb_read(uv_fs_t *req) {
    struct juv_read_state *rs = (struct juv_read_state *)req;
    JanetBuffer *buffer = janet_buffer(0);
    janet_buffer_push_bytes(buffer, (unsigned char *) (rs->bufs[0].base), rs->bufs[0].len);
    uv_fs_req_cleanup(req);
    free(req);
    juv_resume((uv_handle_t *)req, janet_wrap_buffer(buffer), 1);
}
static Janet cfun_fs_read(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 2);
    int fd = getfile(argv, 0);
    int32_t bufn = janet_getinteger(argv, 1);
    if (bufn < 0) {
        janet_panicf("expected postive integer, got %d", bufn);
    }
    struct juv_read_state *read_state = malloc(sizeof(struct juv_read_state) + bufn);
    if (NULL == read_state) janet_panic("no mem.");
    read_state->bufs[0].len = bufn;
    read_state->bufs[0].base = read_state->mem;
    uv_fs_read(uv_default_loop(), &(read_state->req), fd, read_state->bufs, 1, 0, cb_read);
    return janet_wrap_nil();
}

/* DONE */

/* UV open */
/* UV close */
/* UV read */

/* NOT DONE */

/* UV unlink */
/* UV write */
/* UV mkdir */
/* UV mkdtemp */
/* UV rmdir */
/* UV opendir */
/* UV closedir */
/* UV readdir */
/* UV scandir - not needed? */

/* UV stat */
/* UV fstat */
/* UV lstat */

/* UV rename */
/* UV fsync */
/* UV fdatasync */

/* UV ftruncate */
/* UV copyfile */
/* UV sendfile */
/* UV access */

/* UV chmod */
/* UV fchmod */
/* UV utime */
/* UV futime */

/* UV link */
/* UV symlink */
/* UV readlink */
/* UV realpath */

/* UV chown */
/* UV fchown */
/* UV lchown */

static const JanetReg cfuns[] = {
    {"fs/open", cfun_fs_open, NULL},
    {"fs/close", cfun_fs_close, NULL},
    {"fs/read", cfun_fs_read, NULL},
    {NULL, NULL, NULL}
};

void submod_fs(JanetTable *env) {
    janet_cfuns(env, "uv", cfuns);
}
