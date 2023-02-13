#include "libriot/wad.h"

static void
riot_wad_chunk_print(struct riot_wad_chunk *chunk, FILE *f);

void
riot_wad_print(struct riot_wad_ctx *ctx, FILE *f) {
	assert(ctx);
	assert(f);

	fprintf(f, "WAD version %u.%u\n", ctx->wad.major, ctx->wad.minor);
	fprintf(f, "WAD chunks: %u\n", ctx->wad.chunk_count);

	for (u32 i = 0; i < ctx->wad.chunk_count; i++) {
		struct riot_wad_chunk *chunk = ((struct riot_wad_chunk *)ctx->chunk_pool.ptr) + i;
		fprintf(f, "\t");
		riot_wad_chunk_print(chunk, f);
		fprintf(f, "\n");
	}

	fflush(f);
}

static void
riot_wad_chunk_print(struct riot_wad_chunk *chunk, FILE *f) {
	assert(chunk);
	assert(f);

	fprintf(f, "WADChunk(path_hash=0x%08lx,data_offset=0x%08lx,compressed_size=%lu,decompressed_size=%lu,compression=%u,duplicated=%u,sub_chunk_count=%u,sub_chunk_start=%u,checksum=0x%08lx)",
			chunk->path_hash, chunk->data_offset, chunk->compressed_size,
			chunk->decompressed_size, chunk->compression, chunk->duplicated,
			chunk->sub_chunk_count, chunk->sub_chunk_start, chunk->checksum);
}
