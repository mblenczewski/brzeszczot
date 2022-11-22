#ifndef LIBRIOT_IO_H
#define LIBRIOT_IO_H

#include "libriot.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define RIOT_IO_ERROR_FOR_EACH(x) \
	x(RIOT_IO_ERROR_OK) \
	x(RIOT_IO_ERROR_EOF) \
	x(RIOT_IO_ERROR_ALLOC) \
	x(RIOT_IO_ERROR_CORRUPT)

enum riot_io_error {
#define __TOKEN(x) x,
	RIOT_IO_ERROR_FOR_EACH(__TOKEN)
#undef __TOKEN
};

static inline char const *
riot_io_error_str(enum riot_io_error val) {
	switch (val) {
#define __TOKEN(x) case x: return #x;
		RIOT_IO_ERROR_FOR_EACH(__TOKEN)
#undef __TOKEN
		default: return NULL;
	}
}

struct riot_bin_stream {
	struct memslice buf;
	size_t cur;
};

static inline bool
riot_bin_stream_eof(struct riot_bin_stream *self) {
	assert(self);

	return self->buf.len == self->cur;
}

static inline enum riot_io_error
riot_bin_stream_has_remaining(struct riot_bin_stream *self, size_t len) {
	assert(self);

	if ((self->buf.len - self->cur) < len)
		return RIOT_IO_ERROR_EOF;

	return RIOT_IO_ERROR_OK;
}

static inline enum riot_io_error
riot_bin_stream_skip(struct riot_bin_stream *self, size_t len) {
	assert(self);

	enum riot_io_error err = RIOT_IO_ERROR_OK;
	if ((err = riot_bin_stream_has_remaining(self, len)))
		return err;

	self->cur += len;

	return err;
}

static inline enum riot_io_error
riot_bin_stream_peek(struct riot_bin_stream *self, void *buf, size_t len) {
	assert(self);
	assert(buf);

	enum riot_io_error err = RIOT_IO_ERROR_OK;
	if ((err = riot_bin_stream_has_remaining(self, len)))
		return err;

	memcpy(buf, self->buf.ptr + self->cur, len);

	return err;
}

static inline enum riot_io_error
riot_bin_stream_expect(struct riot_bin_stream *self, void *buf, size_t len) {
	assert(self);
	assert(buf);

	enum riot_io_error err = RIOT_IO_ERROR_OK;
	if ((err = riot_bin_stream_has_remaining(self, len)))
		return err;

	if (memcmp(buf, self->buf.ptr + self->cur, len) != 0)
		err = RIOT_IO_ERROR_CORRUPT;

	return err;
}

static inline enum riot_io_error
riot_bin_stream_consume(struct riot_bin_stream *self, void *buf, size_t len) {
	assert(self);
	assert(buf);

	enum riot_io_error err = RIOT_IO_ERROR_OK;
	if ((err = riot_bin_stream_peek(self, buf, len)))
	       return err;

	return riot_bin_stream_skip(self, len);
}

#define RIOT_IO_CHUNK_SIZE 8192

static inline enum riot_io_error
riot_bin_stream_push(struct riot_bin_stream *self, void *buf, size_t len) {
	assert(self);
	assert(buf);

	if (self->buf.len < self->cur + len) {
		size_t new_len = self->buf.len + MAX(RIOT_IO_CHUNK_SIZE, len);
		u8 *new_buf = realloc(self->buf.ptr, new_len * sizeof(u8));
		if (!new_buf) return RIOT_IO_ERROR_ALLOC;

		self->buf.ptr = new_buf;
		self->buf.len = new_len;
	}

	memcpy(self->buf.ptr + self->cur, buf, len);
	self->cur += len;

	return RIOT_IO_ERROR_OK;
}

/* ===========================================================================
 * RIOT INIBIN Format
 * ===========================================================================
 * | -------------------------------------------------------------------------- |
 * | PTCH Header : optional							|
 * | | ================================================================ |	|
 * | | PTCH Magic : 4 bytes, chr8[4]					|	|
 * | | Unknown Bytes : 8 bytes, u64 (flags? size? metadata?)		|	|
 * | -------------------------------------------------------------------------- |
 * | PROP Magic : 4 bytes, chr8[4]						|
 * | Version : 4 bytes, u32							|
 * | -------------------------------------------------------------------------- |
 * | Linked Files : v2+								|
 * | | ================================================================ |	|
 * | | Count : 4 bytes, u32						|	|
 * | | Strings : riot_bin_str[Count]					|	|
 * | -------------------------------------------------------------------------- |
 * | Prop Entries : v1+								|
 * | | ================================================================ |	|
 * | | Count : 4 bytes, u32						|	|
 * | | Entry Name Hashes : fnv1a_u32[Count]				|	|
 * | | Entries[Count]							|	|
 * | | | ====================================================== |	|	|
 * | | | Length : 4 bytes, u32					|	|	|
 * | | | Name Hash : 4 bytes, fnv1a_u32				|	|	|
 * | | | Count : 2 bytes, u16					|	|	|
 * | | | Items : riot_bin_node_field[Count]			|	|	|
 * | | | | ============================================ |	|	|	|
 * | | | | Name Hash : 4 bytes, fnv1a_u32		|	|	|	|
 * | | | | Type : 1 byte, u8				|	|	|	|
 * | | | | Value : riot_bin_node<Type>			|	|	|	|
 * | -------------------------------------------------------------------------- |
 * | Patch Entries : v3+, only if PTCH Header present				|
 * | | ================================================================	|	|
 * | | Count : 4 bytes, u32						|	|
 * | | Entries[Count]							|	|
 * | | | ======================================================	|	|	|
 * | | | Name Hash : 4 bytes, fnv1a_u32				|	|	|
 * | | | Length : 4 bytes, u32					|	|	|
 * | | | Type : 1 byte, u8					|	|	|
 * | | | String : riot_bin_str					|	|	|
 * | | | Value : riot_bin_node<Type>				|	|	|
 * | -------------------------------------------------------------------------- |
 *
 * riot_bin_str:
 * | -------------------------------------------------------------------------- |
 * | Size : 2 bytes, u16							|
 * | Chars : chr8[Size]								|
 * | --------------------------------------------------------------------------	|
 *
 * riot_bin_node Tagged Union:
 * primitives:
 *   b8, u8, s8, u16, s16, u32, s32, u64, s64, f32, fvec2, fvec3, fvec4,
 *   fmat4x4, rgba, fnv1a_u32, xxh64_u64, riot_bin_str, flag_b8
 * pseudo-containers:
 *   ptr, embed:
 *   | ------------------------------------------------------------------------	|
 *   | Name Hash : 4 bytes, fnv1a_u32						|
 *   | Size : 4 bytes, u32							|
 *   | Count : 2 bytes, u16							|
 *   | Items : riot_bin_field[Count]						|
 *   | | ==============================================================	|	|
 *   | | Name Hash : 4 bytes, fnv1a_u32					|	|
 *   | | Type : 1 byte, u8						|	|
 *   | | Value : riot_bin_node<Type>					|	|
 *   | ------------------------------------------------------------------------	|
 * containers:
 *   option:
 *   | ------------------------------------------------------------------------	|
 *   | Type : 1 byte, u8							|
 *   | Count : 1 byte, u8							|
 *   | Value : riot_bin_node<Type>, optional (present only if Count == 1)	|
 *   | ------------------------------------------------------------------------	|
 *   list, list2:
 *   | ------------------------------------------------------------------------	|
 *   | Type : 1 byte, u8							|
 *   | Size : 4 bytes, u32							|
 *   | Count : 4 bytes, u32							|
 *   | Items : riot_bin_node<Type>[Count]					|
 *   | ------------------------------------------------------------------------	|
 *   map:
 *   | ------------------------------------------------------------------------	|
 *   | Key Type : 1 byte, u8							|
 *   | Val Type : 1 byte, u8							|
 *   | Size : 4 bytes, u32							|
 *   | Count : 4 bytes, u32							|
 *   | Items : riot_bin_pair[Count]						|
 *   | | ==============================================================	|	|
 *   | | Key : riot_bin_node<Key Type>					|	|
 *   | | Val : riot_bin_node<Val Type>					|	|
 *   | ------------------------------------------------------------------------	|
 */

enum riot_io_error
riot_bin_try_read(u8 *buf, size_t len, struct riot_bin *out);

enum riot_io_error
riot_bin_try_write(struct riot_bin const *val, u8 **buf, size_t *len);

#define STREAM_ERRLOG(stream) errlog("%s@%zu/%zu: ", __func__, (stream).cur, (stream).buf.len);

#define BIN_ASSERT(cond, errno, cleanup, ...)					\
if (!(cond)) {									\
	errlog("%s:%d:%s: ", __FILE__, __LINE__, __func__);			\
	STREAM_ERRLOG(*stream);							\
	errlog("%s: ", riot_io_error_str(errno));				\
	errlog(__VA_ARGS__);							\
	errlog("\n");								\
	err = errno;								\
	goto cleanup;								\
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBRIOT_IO_H */
