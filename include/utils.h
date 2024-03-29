#ifndef UTILS_H
#define UTILS_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct str_view {
	char *ptr;
	u64 len;
};

static inline bool
str_view_equal(struct str_view a, struct str_view b) {
	return a.len == b.len && strncmp(a.ptr, b.ptr, a.len) == 0;
}

#define CONST_CSTR_SV(ccstr) \
	((struct str_view){ .ptr = ccstr, .len = sizeof(ccstr) - 1, })

#define CSTR_SV(cstr) \
	((struct str_view){ .ptr = cstr, .len = strlen(cstr), })

struct mem_stream {
	u8 *ptr;
	u64 cur, len;
};

static inline bool
mem_stream_eof(struct mem_stream *self) {
	assert(self);

	return self->len == self->cur;
}

static inline u8 *
mem_stream_headptr(struct mem_stream *self) {
	assert(self);

	return self->ptr + self->cur;
}

static inline bool
mem_stream_skip(struct mem_stream *self, u64 len) {
	assert(self);

	if (self->len - self->cur < len)
		return false;

	self->cur += len;

	return true;
}

static inline bool
mem_stream_peek(struct mem_stream *self, u64 off, void *buf, u64 len) {
	assert(self);
	assert(buf);

	if (self->len - off - self->cur < len)
		return false;

	memcpy(buf, self->ptr + self->cur + off, len);

	return true;
}

static inline bool
mem_stream_consume(struct mem_stream *self, void *buf, u64 len) {
	assert(self);
	assert(buf);

	return mem_stream_peek(self, 0, buf, len) && mem_stream_skip(self, len);
}

static inline bool
mem_stream_resize(struct mem_stream *self, u64 capacity) {
	assert(self);

	u8 *ptr = realloc(self->ptr, capacity);
	if (!ptr) return false;

	self->ptr = ptr;
	self->len = capacity;

	return true;
}

static inline u64
mem_stream_push(struct mem_stream *self, void *buf, u64 len) {
	assert(self);
	assert(buf);

	if (self->len - self->cur < len && !mem_stream_resize(self, self->cur + len))
		return 0;

	memcpy(self->ptr + self->cur, buf, len);
	self->cur += len;

	return len;
}

struct mem_pool {
	u8 *ptr;
	u64 cap, len;
};

static inline bool
mem_pool_resize(struct mem_pool *self, u64 alignment, u64 capacity) {
	assert(self);

#ifdef _WIN32
	u8 *ptr = _aligned_realloc(self->ptr, capacity, alignment);
#else
	u8 *ptr = realloc(self->ptr, capacity);
#endif
	if (!ptr) return false;

	self->ptr = ptr;
	self->cap = capacity;

	return true;
}

static inline bool
mem_pool_init(struct mem_pool *self, u64 alignment, u64 capacity) {
	assert(self);
	assert(alignment);
	assert(alignment == 1 || alignment % 2 == 0);
	assert(capacity % alignment == 0);

#ifdef _WIN32
	u8 *ptr = _aligned_malloc(capacity, alignment);
#else
	u8 *ptr = aligned_alloc(alignment, capacity);
#endif
	if (!ptr) return false;

	self->ptr = ptr;
	self->cap = capacity;
	self->len = 0;

	return true;
}

#define MEM_POOL_INIT(pool, type, capacity) \
mem_pool_init((pool), alignof(type), (capacity) * sizeof(type))

static inline void
mem_pool_free(struct mem_pool *self) {
	assert(self);

#ifdef _WIN32
	_aligned_free(self->ptr);
#else
	free(self->ptr);
#endif
}

static inline void
mem_pool_reset(struct mem_pool *self) {
	assert(self);

	self->len = 0;
}

static inline bool
mem_pool_prealloc(struct mem_pool *self, u64 alignment, u64 size) {
	assert(self);

	return self->len + size <= self->cap ||
		mem_pool_resize(self, alignment, self->len + size);
}

static inline void *
mem_pool_alloc(struct mem_pool *self, u64 alignment, u64 size) {
	assert(self);
	assert(alignment);
	assert(alignment == 1 || alignment % 2 == 0);
	assert(size % alignment == 0);

	u64 alignment_off = alignment - 1;
	u64 aligned_len = (self->len + alignment_off) & ~alignment_off;

	if (!mem_pool_prealloc(self, alignment, (aligned_len - self->len) + size))
		return NULL;

	void *ptr = self->ptr + aligned_len;
	self->len = aligned_len + size;

	return ptr;
}

#define MEM_POOL_ALLOC(pool, type, count) \
(type *)mem_pool_alloc((pool), alignof(type), (count) * sizeof(type))

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* UTILS_H */
