#ifndef LIBRIOT_UTILS_H
#define LIBRIOT_UTILS_H

#include "common.h"
#include "utils.h"

#include "libriot.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* extension methods to `struct mem_stream` to read riot primitives
 */

extern b32
riot_mem_stream_read_chr8(struct mem_stream *self, char *out);

extern b32
riot_mem_stream_read_b8(struct mem_stream *self, b8 *out);

extern b32
riot_mem_stream_read_s8(struct mem_stream *self, s8 *out);

extern b32
riot_mem_stream_read_s16(struct mem_stream *self, s16 *out);

extern b32
riot_mem_stream_read_s32(struct mem_stream *self, s32 *out);

extern b32
riot_mem_stream_read_s64(struct mem_stream *self, s64 *out);

extern b32
riot_mem_stream_read_u8(struct mem_stream *self, u8 *out);

extern b32
riot_mem_stream_read_u16(struct mem_stream *self, u16 *out);

extern b32
riot_mem_stream_read_u32(struct mem_stream *self, u32 *out);

extern b32
riot_mem_stream_read_u64(struct mem_stream *self, u64 *out);

extern b32
riot_mem_stream_read_fnv1a_u32(struct mem_stream *self, fnv1a_u32 *out);

extern b32
riot_mem_stream_read_xxh64_u64(struct mem_stream *self, xxh64_u64 *out);

/* extension methods to `struct mem_stream` to write riot primitives
 */

extern b32
riot_mem_stream_write_chr8(struct mem_stream *self, char val);

extern b32
riot_mem_stream_write_b8(struct mem_stream *self, b8 val);

extern b32
riot_mem_stream_write_s8(struct mem_stream *self, s8 val);

extern b32
riot_mem_stream_write_s16(struct mem_stream *self, s16 val);

extern b32
riot_mem_stream_write_s32(struct mem_stream *self, s32 val);

extern b32
riot_mem_stream_write_s64(struct mem_stream *self, s64 val);

extern b32
riot_mem_stream_write_u8(struct mem_stream *self, u8 val);

extern b32
riot_mem_stream_write_u16(struct mem_stream *self, u16 val);

extern b32
riot_mem_stream_write_u32(struct mem_stream *self, u32 val);

extern b32
riot_mem_stream_write_u64(struct mem_stream *self, u64 val);

extern b32
riot_mem_stream_write_fnv1a_u32(struct mem_stream *self, fnv1a_u32 val);

extern b32
riot_mem_stream_write_xxh64_u64(struct mem_stream *self, xxh64_u64 val);

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* LIBRIOT_UTILS_H */
