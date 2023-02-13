#include "libriot/wad.h"

static b32
riot_wad_chunk_read(struct riot_wad_ctx *ctx, struct mem_stream *stream, struct riot_wad_chunk *chunk);

b32
riot_wad_read(struct riot_wad_ctx *ctx, struct mem_stream stream) {
	assert(ctx);

	char magic[2] = { 'R', 'W', }, buf[sizeof(magic)];
	if (!mem_stream_consume(&stream, buf, sizeof magic)) {
		errlog("Failed to read WAD magic");
		return false;
	}

	if (memcmp(magic, buf, sizeof magic) != 0) {
		errlog("Bad WAD magic value: %c%c", buf[0], buf[1]);
		return false;
	}

	if (!riot_mem_stream_read_u8(&stream, &ctx->wad.major)) {
		errlog("Failed to read WAD major version");
		return false;
	}

	if (!riot_mem_stream_read_u8(&stream, &ctx->wad.minor)) {
		errlog("Failed to read WAD minor version");
		return false;
	}

	dbglog("WAD Version: %u.%u", ctx->wad.major, ctx->wad.minor);

	switch (ctx->wad.major) {
	case 1:
		break;

	case 2: {
		u32 ecdsa_signature_length;
		if (!riot_mem_stream_read_u32(&stream, &ecdsa_signature_length)) {
			errlog("Failed to read WAD v2 signature length")
			return false;
		}

		if (!mem_stream_skip(&stream, ecdsa_signature_length)) {
			errlog("Failed to read WAD v2 signature (%u bytes)", ecdsa_signature_length);
			return false;
		}

		u64 checksum;
		if (!riot_mem_stream_read_u64(&stream, &checksum)) {
			errlog("Failed to read WAD v2 signature checksum");
			return false;
		}

		dbglog("WAD v2 signature: length: %u, checksum: %lu", ecdsa_signature_length, checksum);

		// TODO: check signature and checksum values
	} break;

	case 3: {
		u32 ecdsa_signature_length = 256;
		if (!mem_stream_skip(&stream, ecdsa_signature_length)) {
			errlog("Failed to read WAD v3 signature (%u bytes)", ecdsa_signature_length);
			return false;
		}

		u64 checksum;
		if (!riot_mem_stream_read_u64(&stream, &checksum)) {
			errlog("Failed to read WAD v3 signature checksum");
			return false;
		}

		dbglog("WAD v3 signature: length: %u, checksum: %lu", ecdsa_signature_length, checksum);

		// TODO: check signature and checksum values
	} break;

	default:
		unreachable("Unknown WAD major version: %u", ctx->wad.major);
		break;
	}

	if (ctx->wad.major <= 2) {
		u16 toc_offset;
		if (!riot_mem_stream_read_u16(&stream, &toc_offset)) {
			errlog("Failed to read WAD table of contents offset");
			return false;
		}

		u16 toc_entry_size;
		if (!riot_mem_stream_read_u16(&stream, &toc_entry_size)) {
			errlog("Failed to read WAD table of contents entry size");
			return false;
		}

		dbglog("WAD TOC: offset: %u, entry size: %u", toc_offset, toc_entry_size);

		// TODO: handle table of contents
	}

	if (!riot_mem_stream_read_u32(&stream, &ctx->wad.chunk_count)) {
		errlog("Failed to read WAD chunk count");
		return false;
	}

	dbglog("WAD Chunks: %u", ctx->wad.chunk_count);

	riot_offptr_t chunk_offptr;
	if (!riot_wad_ctx_pushn_chunk(ctx, ctx->wad.chunk_count, &chunk_offptr)) {
		errlog("Failed to preallocate %u WAD chunks");
		return false;
	}

	(void) chunk_offptr;

	for (u32 i = 0; i < ctx->wad.chunk_count; i++) {
		struct riot_wad_chunk *chunk = (struct riot_wad_chunk *)ctx->chunk_pool.ptr + i;
		if (!riot_wad_chunk_read(ctx, &stream, chunk)) {
			errlog("Failed to read WAD chunk %u/%u", i + 1, ctx->wad.chunk_count);
			return false;
		}
	}

	dbglog("Read %u WAD chunks", ctx->wad.chunk_count);
	dbglog("WAD chunk segment end: %lu", stream.cur, stream.len);
	dbglog("WAD data segment start: %lu", ctx->wad.data_start);
	dbglog("WAD ctx chunk pool size: %lu/%lu bytes", ctx->chunk_pool.len, ctx->chunk_pool.cap);

	return true;
}

static b32
riot_wad_chunk_read(struct riot_wad_ctx *ctx, struct mem_stream *stream, struct riot_wad_chunk *chunk) {
	assert(ctx);
	assert(stream);
	assert(chunk);

	if (!riot_mem_stream_read_xxh64_u64(stream, &chunk->path_hash)) {
		errlog("Failed to read WAD chunk path hash");
		return false;
	}

	if (!riot_mem_stream_read_u32(stream, &chunk->data_offset)) {
		errlog("Failed to read WAD chunk data offset");
		return false;
	}

	if (chunk->data_offset < ctx->wad.data_start)
		ctx->wad.data_start = chunk->data_offset;

	if (!riot_mem_stream_read_u32(stream, &chunk->compressed_size)) {
		errlog("Failed to read WAD chunk compressed data size");
		return false;
	}

	if (!riot_mem_stream_read_u32(stream, &chunk->decompressed_size)) {
		errlog("Failed to read WAD chunk decompressed data size");
		return false;
	}

	u8 sub_chunk_count_and_compression_type;
	if (!riot_mem_stream_read_u8(stream, &sub_chunk_count_and_compression_type)) {
		errlog("Failed to read WAD chunk sub-chunk count and compression type byte");
		return false;
	}

	chunk->compression = (enum riot_wad_compression)sub_chunk_count_and_compression_type & 0xf;
	chunk->sub_chunk_count = sub_chunk_count_and_compression_type >> 4;

	if (!riot_mem_stream_read_b8(stream, &chunk->duplicated)) {
		errlog("Failed to read WAD chunk duplication flag");
		return false;
	}

	if (!riot_mem_stream_read_u16(stream, &chunk->sub_chunk_start)) {
		errlog("Failed to read WAD sub-chunk start");
		return false;
	}

	if (ctx->wad.major > 2) {
		if (!riot_mem_stream_read_u64(stream, &chunk->checksum)) {
			errlog("Failed to read WAD chunk checksum");
			return false;
		}
	}

	return true;
}
