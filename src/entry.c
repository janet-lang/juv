#include "entry.h"

int juv_last_error = 0;

/* Embedded source */
extern const unsigned char *embed___entry_embed;
extern size_t embed___entry_embed_size;

Janet cfun_run(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 0);
    (void) argv;
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    uv_loop_close(uv_default_loop());
    return janet_wrap_nil();
}

Janet cfun_checkerror(int32_t argc, Janet *argv) {
    janet_fixarity(argc, 0);
    (void) argv;
    if (juv_last_error < 0) {
        return janet_cstringv(uv_strerror(juv_last_error));
    } else {
        return janet_wrap_nil();
    }
}

static const JanetReg cfuns[] = {
    {"run", cfun_run,
        "(uv/run)\n\n"
        "Run the main uv loop."
    },
    {"check-error", cfun_checkerror, NULL},
    {NULL, NULL, NULL}
};

JANET_MODULE_ENTRY(JanetTable *env) {
    janet_cfuns(env, "uv", cfuns);
    janet_def(env, "version", janet_cstringv(uv_version_string()),
            "Libuv version string as a semantic version.");
    submod_fs(env);
    submod_timer(env);
    /* Embed */
    janet_dobytes(env, embed___entry_embed, embed___entry_embed_size, "[uv/entry.janet]", NULL);
}
