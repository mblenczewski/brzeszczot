#include "libriot/utils.h"

#define READ_BYTES(stream, type, out) \
do { \
	type _tmp = 0; \
	u8 buf; \
	for (u64 i = 0; i < sizeof(type); i++) { \
		if (!mem_stream_consume(stream, &buf, 1)) return false; \
		_tmp |= ((type)buf << (i * 8)); \
	} \
	*out = _tmp; \
} while (0);

#define MAKE_READ_FN(name, type) \
b32 name(struct mem_stream *self, type *out) { \
	assert(self); \
	assert(out); \
	READ_BYTES(self, type, out) \
	return true; \
}

MAKE_READ_FN(riot_mem_stream_read_chr8, char)
MAKE_READ_FN(riot_mem_stream_read_b8, b8)
MAKE_READ_FN(riot_mem_stream_read_s8, s8)
MAKE_READ_FN(riot_mem_stream_read_s16, s16)
MAKE_READ_FN(riot_mem_stream_read_s32, s32)
MAKE_READ_FN(riot_mem_stream_read_s64, s64)
MAKE_READ_FN(riot_mem_stream_read_u8, u8)
MAKE_READ_FN(riot_mem_stream_read_u16, u16)
MAKE_READ_FN(riot_mem_stream_read_u32, u32)
MAKE_READ_FN(riot_mem_stream_read_u64, u64)
MAKE_READ_FN(riot_mem_stream_read_fnv1a_u32, fnv1a_u32)
MAKE_READ_FN(riot_mem_stream_read_xxh64_u64, xxh64_u64)

#define WRITE_BYTES(stream, type, val) \
do { \
	u8 buf; \
	for (u64 i = 0; i < sizeof(type); i++) { \
		buf = ((val >> (i * 8)) & 0xff); \
		if (!mem_stream_push(stream, &buf, 1)) return false; \
	} \
} while (0);

#define MAKE_WRITE_FN(name, type) \
b32 name(struct mem_stream *self, type val) { \
	assert(self); \
	WRITE_BYTES(self, type, val) \
	return true; \
}

MAKE_WRITE_FN(riot_mem_stream_write_chr8, char)
MAKE_WRITE_FN(riot_mem_stream_write_b8, b8)
MAKE_WRITE_FN(riot_mem_stream_write_s8, s8)
MAKE_WRITE_FN(riot_mem_stream_write_s16, s16)
MAKE_WRITE_FN(riot_mem_stream_write_s32, s32)
MAKE_WRITE_FN(riot_mem_stream_write_s64, s64)
MAKE_WRITE_FN(riot_mem_stream_write_u8, u8)
MAKE_WRITE_FN(riot_mem_stream_write_u16, u16)
MAKE_WRITE_FN(riot_mem_stream_write_u32, u32)
MAKE_WRITE_FN(riot_mem_stream_write_u64, u64)
MAKE_WRITE_FN(riot_mem_stream_write_fnv1a_u32, fnv1a_u32)
MAKE_WRITE_FN(riot_mem_stream_write_xxh64_u64, xxh64_u64)
