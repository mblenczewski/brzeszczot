#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
	#include <cassert>
	#include <cerrno>
	#include <cfloat>
	#include <climits>
	#include <csetjmp>
	#include <cstdalign>
	#include <cstdatomic>
	#include <cstdarg>
	#include <cstdbool>
	#include <cstddef>
	#include <cstdint>
	#include <cstdio>
	#include <cstdlib>
	#include <cstdnoreturn>
	#include <cstring>
#else
	#include <assert.h>
	#include <errno.h>
	#include <float.h>
	#include <limits.h>
	#include <setjmp.h>
	#include <stdalign.h>
	#include <stdatomic.h>
	#include <stdarg.h>
	#include <stdbool.h>
	#include <stddef.h>
	#include <stdint.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <stdnoreturn.h>
	#include <string.h>
#endif

typedef int32_t b32;

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

#define KiB 1024ULL
#define MiB 1024 * KiB
#define GiB 1024 * MiB

#define ARRLEN(arr) (sizeof(arr) / sizeof((arr)[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define BITS_SET(value, mask) (((value) & mask) == (mask))

#define RELPTR_MASK(relptr_ty) ((relptr_ty)1 << ((sizeof(relptr_ty) * 8) - 1))
#define RELPTR_NULL (0)

#define RELPTR_ENC(relptr_ty, ptroff) \
	((relptr_ty)((ptroff) ^ RELPTR_MASK(relptr_ty)))

#define RELPTR_DEC(relptr_ty, relptr) \
	((relptr_ty)((relptr) ^ RELPTR_MASK(relptr_ty)))

#define RELPTR_ABS2REL(relptr_ty, base, absptr) \
	((absptr) \
	? RELPTR_ENC(relptr_ty, (u8*)absptr - (u8*)base) \
	: RELPTR_NULL)

#define RELPTR_REL2ABS(absptr_ty, relptr_ty, base, relptr) \
	((relptr) \
	? ((absptr_ty)((u8*)base + RELPTR_DEC(relptr_ty, relptr))) \
	: NULL)

#define errloc() fprintf(stderr, "%s:%d:%s: ", __FILE__, __LINE__, __func__);

#define errlog(...) \
do { \
	errloc(); \
	fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\n"); \
	fflush(stderr); \
} while (0);

#define unreachable(...) \
do { \
	errloc(); \
	fprintf(stderr, "UNREACHABLE: "); \
	fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\n"); \
	fflush(stderr); \
	abort(); \
} while (0);

#define unimplemented(...) \
do { \
	errloc(); \
	fprintf(stderr, "UNIMPLEMENTED: "); \
	fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\n"); \
	fflush(stderr); \
	abort(); \
} while (0);

#ifndef NDEBUG
	#define dbglog(...) errlog(__VA_ARGS__)
#else
	#define dbglog(...)
#endif

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* COMMON_H */
