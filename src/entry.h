#ifndef ENTRY_H_V7RZOD03
#define ENTRY_H_V7RZOD03

#include <janet.h>
#include <uv.h>

/* The header for all components of the project. */

/* Main libuv functions */
Janet cfun_init(int32_t argc, Janet *argv);
Janet cfun_deinit(int32_t argc, Janet *argv);
Janet cfun_run(int32_t argc, Janet *argv);
Janet cfun_checkerror(int32_t argc, Janet *argv);

/* Utilities */
void juv_panic(int r);
void juv_schedule(uv_handle_t *handle);
void juv_resume(uv_handle_t *handle, Janet value, int freemem);
void juv_toperror(JanetSignal sig, Janet out);
int juv_parse_flags(const uint8_t *arg, const char *chars, const int *flags);
extern int juv_last_error;

/* submodules */
void submod_fs(JanetTable *env);
void submod_timer(JanetTable *env);

#endif /* end of include guard: ENTRY_H_V7RZOD03 */
