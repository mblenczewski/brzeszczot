#include "libriot/wad.h"

static b32
riot_wad_chunk_write(struct riot_wad_ctx *ctx, struct riot_wad_chunk *chunk, struct mem_stream *stream);

b32
riot_wad_write(struct riot_wad_ctx *ctx, void *data, u64 len, struct mem_stream stream) {
	assert(ctx);

	char magic[2] = { 'R', 'W', };
	if (!mem_stream_push(&stream, magic, sizeof magic)) {
		errlog("Failed to write WAD magic");
		return false;
	}

	if (!riot_mem_stream_write_u8(&stream, 3)) {
		errlog("Failed to write WAD version major");
		return false;
	}

	if (!riot_mem_stream_write_u8(&stream, 1)) {
		errlog("Failed to write WAD version minor");
		return false;
	}

	u32 ecdsa_signature_length = 256;
	u8 signature[ecdsa_signature_length];
	memset(signature, 0, ecdsa_signature_length);
	if (!mem_stream_push(&stream, signature, ecdsa_signature_length)) {
		errlog("Failed to write v3 signature (%u bytes)", ecdsa_signature_length);
		return false;
	}

	u64 checksum = 0;
	if (!riot_mem_stream_write_u64(&stream, checksum)) {
		errlog("Failed to write v3 signature checksum");
		return false;
	}

	if (!riot_mem_stream_write_u32(&stream, ctx->wad.chunk_count)) {
		errlog("Failed to write WAD chunk count");
		return false;
	}

	for (u32 i = 0; i < ctx->wad.chunk_count; i++) {
		struct riot_wad_chunk *chunk = (struct riot_wad_chunk *)ctx->chunk_pool.ptr + i;
		if (!riot_wad_chunk_write(ctx, chunk, &stream)) {
			errlog("Failed to write WAD chunk %u/%u", i + 1, ctx->wad.chunk_count);
			return false;
		}
	}

	if (!mem_stream_push(&stream, data, len)) {
		errlog("Failed to write WAD data segment");
		return false;
	}

	return true;
}

static b32
riot_wad_chunk_write(struct riot_wad_ctx *ctx, struct riot_wad_chunk *chunk, struct mem_stream *stream) {
	assert(ctx);
	assert(chunk);
	assert(stream);

	if (!riot_mem_stream_write_xxh64_u64(stream, chunk->path_hash)) {
		errlog("Failed to write WAD chunk path hash");
		return false;
	}

	if (!riot_mem_stream_write_u32(stream, chunk->data_offset)) {
		errlog("Failed to write WAD chunk data offset");
		return false;
	}

	if (!riot_mem_stream_write_u32(stream, chunk->compressed_size)) {
		errlog("Failed to write WAD chunk compressed data size");
		return false;
	}

	if (!riot_mem_stream_write_u32(stream, chunk->decompressed_size)) {
		errlog("Failed to write WAD chunk decompressed data size");
		return false;
	}

	u8 sub_chunk_count_and_compression_type = 0;
	sub_chunk_count_and_compression_type |= (chunk->sub_chunk_count << 4);
	sub_chunk_count_and_compression_type |= (u8)chunk->compression & 0xf;
	if (!riot_mem_stream_write_u8(stream, sub_chunk_count_and_compression_type)) {
		errlog("Failed to write WAD chunk sub-chunk count and compression type byte");
		return false;
	}

	if (!riot_mem_stream_write_b8(stream, chunk->duplicated)) {
		errlog("Failed to write WAD chunk duplicated flag");
		return false;
	}

	if (!riot_mem_stream_write_u16(stream, chunk->sub_chunk_start)) {
		errlog("Failed to write WAD chunk sub-chunk start");
		return false;
	}

	u64 checksum = 0;
	if (!riot_mem_stream_write_u64(stream, checksum)) {
		errlog("Failed to write WAD chunk checksum");
		return false;
	}

	return true;
}
