#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <assert.h>
#include <float.h>
#include <limits.h>
#include <setjmp.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdnoreturn.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

#define ALLOC_OVERFLOWS(type, count) ((SIZE_MAX / sizeof(type)) < (count))
#define ARRLEN(arr) (sizeof(arr) / sizeof((arr)[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define BITS_SET(v, m) (((v) & (m)) == (m))

#define errlog(...) fprintf(stderr, __VA_ARGS__)

#ifndef NDEBUG
	#define dbglog(...) errlog(__VA_ARGS__)
#else
	#define dbglog(...)
#endif

struct memslice {
	u8 *ptr;
	size_t len;
};

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* COMMON_H */
