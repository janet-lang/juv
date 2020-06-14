#ifndef STREAM_H_ATUIFR2S
#define STREAM_H_ATUIFR2S

#include <uv.h>
#include "entry.h"

extern const JanetAbstractType tcp_type;
extern const JanetAbstractType pipe_type;
extern const JanetAbstractType tty_type;

uv_stream_t *juv_getstream(const Janet *argv, int32_t n);

/* Stream methods */
Janet juv_shutdown(int32_t argc, Janet *argv);
Janet juv_listen(int32_t argc, Janet *argv);
Janet juv_accept(int32_t argc, Janet *argv);

Janet juv_read_start(int32_t argc, Janet *argv);
Janet juv_read_stop(int32_t argc, Janet *argv);

Janet juv_write(int32_t argc, Janet *argv);
Janet juv_try_write(int32_t argc, Janet *argv);

Janet juv_is_readable(int32_t argc, Janet *argv);
Janet juv_is_writeable(int32_t argc, Janet *argv);
Janet juv_stream_get_write_queue_size(int32_t argc, Janet *argv);

#endif /* end of include guard: STREAM_H_ATUIFR2S */
