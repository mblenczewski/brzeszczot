#ifndef LIBRIOT_WAD_H
#define LIBRIOT_WAD_H

#include "common.h"
#include "utils.h"

#include "libriot.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum riot_wad_compression {
	RIOT_WAD_COMPRESSION_NONE	= 0,
	RIOT_WAD_COMPRESSION_GZIP	= 1,
	RIOT_WAD_COMPRESSION_SATELLITE	= 2,
	RIOT_WAD_COMPRESSION_ZSTD	= 3,
	RIOT_WAD_COMPRESSION_ZSTD_CHUNK	= 4,
};

struct riot_wad_chunk {
	xxh64_u64 path_hash;
	u32 data_offset, compressed_size, decompressed_size;
	enum riot_wad_compression compression;
	b8 duplicated;
	u16 sub_chunk_count, sub_chunk_start;
	u64 checksum;
	struct riot_intrusive_list_node list;
};

struct riot_wad {
	u8 major, minor;
	riot_offptr_t root_chunk;
};

#define RIOT_WAD_CTX_CHUNK_POOL_SZ 4 * KiB

struct riot_wad_ctx {
	struct mem_pool chunk_pool;
};

extern b32
riot_wad_ctx_init(struct riot_wad_ctx *self);

extern void
riot_wad_ctx_free(struct riot_wad_ctx *self);

extern b32
riot_wad_ctx_pushn_chunk(struct riot_wad_ctx *self, u32 count, riot_offptr_t *out);

extern b32
riot_wad_read(struct riot_wad_ctx *ctx, struct mem_stream stream);

extern b32
riot_wad_write(struct riot_wad_ctx *ctx, struct mem_stream stream);

extern void
riot_wad_print(struct riot_wad_ctx *ctx, FILE *f);

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* LIBRIOT_WAD_H */
