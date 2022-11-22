#include "libriot/io.h"

static enum riot_io_error
riot_bin_size_node(struct riot_bin_stream *stream,
			enum riot_bin_node_type type,
			struct riot_bin_mempool *mempool);

static enum riot_io_error
riot_bin_size_linked_files(struct riot_bin_stream *stream,
			struct riot_bin_mempool *mempool);

static enum riot_io_error
riot_bin_size_entries(struct riot_bin_stream *stream,
			struct riot_bin_mempool *mempool);

static enum riot_io_error
riot_bin_size_patches(struct riot_bin_stream *stream,
			struct riot_bin_mempool *mempool);

enum riot_io_error
riot_bin_size(u8 *buf, size_t len, struct riot_bin_mempool *out) {
	assert(buf);
	assert(out);

	struct riot_bin_stream _stream = {
		.buf = {
			.ptr = buf,
			.len = len,
		},
		.cur = 0,
	}, *stream = &_stream;

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	u8 magic[4] = {0}, patch_magic[4] = "PTCH", prop_magic[4] = "PROP";
	bool has_patches = false;

	struct riot_bin_mempool mempool = {0};

	err = riot_bin_stream_consume(stream, magic, sizeof magic);
	BIN_ASSERT(!err, err, failure, "Failed to read magic value!")

	/* if we read the patch magic, then we need to skip ahead 8 bytes to
	 * skip past the unknown 8 bytes remaining in the patch header and
	 * re-read the magic (as we expect the PROP magic identifier)
	 */
	if (memcmp(magic, patch_magic, sizeof magic) == 0) {
		has_patches = true;

		err = riot_bin_stream_skip(stream, 8);
		BIN_ASSERT(!err, err, failure, "Failed to skip past patch header!")

		err = riot_bin_stream_consume(stream, magic, sizeof magic);
		BIN_ASSERT(!err, err, failure, "Failed to read magic value!")
	}

	/* if we don't successfully read the patch magic identifier or
	 * the prop magic identifier then we have a corrupted bin file
	 */
	if (memcmp(magic, prop_magic, sizeof magic) != 0) {
		BIN_ASSERT(false, RIOT_IO_ERROR_CORRUPT, failure, "Invalid magic value read!");
	}

	u32 version = 0;
	err = riot_bin_stream_consume(stream, &version, sizeof(version));
	BIN_ASSERT(!err, err, failure, "Failed to read version identifier!")

	if (version >= 2) {
		err = riot_bin_size_linked_files(stream, &mempool);
		BIN_ASSERT(!err, err, failure, "Failed to size linked files!")
	}

	err = riot_bin_size_entries(stream, &mempool);
	BIN_ASSERT(!err, err, failure, "Failed to size prop entries!")

	if (version >= 3 && has_patches) {
		err = riot_bin_size_patches(stream, &mempool);
		BIN_ASSERT(!err, err, failure, "Failed to size patch entries!");
	}

	BIN_ASSERT(riot_bin_stream_eof(stream), RIOT_IO_ERROR_CORRUPT, failure, "Stream input remaining!")

	*out = mempool;

failure:
	return err;
}

static inline enum riot_io_error
riot_bin_size_linked_files(struct riot_bin_stream *stream, struct riot_bin_mempool *mempool) {
	assert(stream);
	assert(mempool);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	u32 count = 0;
	err = riot_bin_stream_consume(stream, &count, sizeof(count));
	BIN_ASSERT(!err, err, failure, "Failed to read number of linked files!")

	if (!count) return err;

	for (u32 i = 0; i < count; i++) {
		err = riot_bin_size_node(stream, RIOT_BIN_NODE_TYPE_STR, mempool);
		BIN_ASSERT(!err, err, failure, "Failed to size linked files %u!", i)
	}

failure:
	return err;
}

static enum riot_io_error
riot_bin_size_entry(struct riot_bin_stream *stream, struct riot_bin_mempool *mempool) {
	assert(stream);
	assert(mempool);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	u32 size = 0;
	err = riot_bin_stream_consume(stream, &size, sizeof(size));
	BIN_ASSERT(!err, err, failure, "Failed to read total size of prop entry embed!")

	size_t prev_cur = stream->cur;

	err = riot_bin_stream_skip(stream, sizeof(hashes_fnv1a_val_t));
	BIN_ASSERT(!err, err, failure, "Failed to skip prop entry embed name hash!")

	u16 count = 0; 
	err = riot_bin_stream_consume(stream, &count, sizeof(count));
	BIN_ASSERT(!err, err, failure, "Failed to read count of prop entry embed fields!")

	for (u16 i = 0; i < count; i++) {
		err = riot_bin_stream_skip(stream, sizeof(hashes_fnv1a_val_t));
		BIN_ASSERT(!err, err, failure, "Failed to skip prop entry embed's field %u name hash!", i)

		u8 raw_type;
		err = riot_bin_stream_consume(stream, &raw_type, sizeof(raw_type));
		BIN_ASSERT(!err, err, failure, "Failed to read prop entry embed's field %u type!", i)

		enum riot_bin_node_type type = riot_bin_node_type_from_raw(raw_type);
		err = riot_bin_size_node(stream, type, mempool);
		BIN_ASSERT(!err, err, failure, "Failed to size prop entry embed's field %u value!", i)
	}

	BIN_ASSERT(stream->cur - prev_cur == size, RIOT_IO_ERROR_CORRUPT, failure,
			"Prop entry embed parsing failed (%zu bytes read, %u bytes expected)!", stream->cur - prev_cur, size)

	mempool->fields.len += count;

failure:
	return err;
}

static inline enum riot_io_error
riot_bin_size_entries(struct riot_bin_stream *stream, struct riot_bin_mempool *mempool) {
	assert(stream);
	assert(mempool);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	u32 count = 0;
	err = riot_bin_stream_consume(stream, &count, sizeof(count));
	BIN_ASSERT(!err, err, failure, "Failed to read number of prop entries!")

	if (!count) return err;

	err = riot_bin_stream_skip(stream, count * sizeof(hashes_fnv1a_val_t));
	BIN_ASSERT(!err, err, failure, "Failed to skip over prop entry name hashes!")

	for (u32 i = 0; i < count; i++) {
		err = riot_bin_size_entry(stream, mempool);
		BIN_ASSERT(!err, err, failure, "Failed to size embed entry %u!", i)
	}

	mempool->pairs.len += count;

failure:
	return err;
}

static enum riot_io_error
riot_bin_size_patch(struct riot_bin_stream *stream, struct riot_bin_mempool *mempool) {
	assert(stream);
	assert(mempool);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	err = riot_bin_stream_skip(stream, sizeof(hashes_fnv1a_val_t));
	BIN_ASSERT(!err, err, failure, "Failed to skip patch entry embed name hash!")

	u32 size = 0;
	err = riot_bin_stream_consume(stream, &size, sizeof(size));
	BIN_ASSERT(!err, err, failure, "Failed to read total size of patch entry embed!")

	size_t prev_cur = stream->cur;

	u8 raw_type;
	err = riot_bin_stream_consume(stream, &raw_type, sizeof(raw_type));
	BIN_ASSERT(!err, err, failure, "Failed to read raw type of patch entry embed value!");

	enum riot_bin_node_type type = riot_bin_node_type_to_raw(raw_type);

	err = riot_bin_size_node(stream, RIOT_BIN_NODE_TYPE_STR, mempool);
	BIN_ASSERT(!err, err, failure, "Failed to size patch entry embed path!")

	err = riot_bin_size_node(stream, type, mempool);
	BIN_ASSERT(!err, err, failure, "Failed to size patch entry embed value!")

	BIN_ASSERT(stream->cur - prev_cur == size, RIOT_IO_ERROR_ALLOC, failure,
			"Patch entry embed parsing failed (%zu bytes read, %u bytes expected)!", stream->cur - prev_cur, size)

	mempool->fields.len += 2;

failure:
	return err;
}

static inline enum riot_io_error
riot_bin_size_patches(struct riot_bin_stream *stream, struct riot_bin_mempool *mempool) {
	assert(stream);
	assert(mempool);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	u32 count = 0;
	err = riot_bin_stream_consume(stream, &count, sizeof(count));
	BIN_ASSERT(!err, err, failure, "Failed to read number of patches!")

	if (!count) return err;

	for (u32 i = 0; i < count; i++) {
		err = riot_bin_size_patch(stream, mempool);
		BIN_ASSERT(!err, err, failure, "Failed to size patch %u!", i)
	}

	mempool->pairs.len += count;

failure:
	return err;
}

static enum riot_io_error
riot_bin_read_node(struct riot_bin_stream *stream,
			enum riot_bin_node_type type,
			struct riot_bin_node *out,
			struct riot_bin_mempool *mempool);

static enum riot_io_error
riot_bin_read_linked_files(struct riot_bin_stream *stream, struct riot_bin *out);

static enum riot_io_error
riot_bin_read_entries(struct riot_bin_stream *stream, struct riot_bin *out);

static enum riot_io_error
riot_bin_read_patches(struct riot_bin_stream *stream, struct riot_bin *out);

enum riot_io_error
riot_bin_try_read(u8 *buf, size_t len, struct riot_bin *out) {
	assert(buf);
	assert(out);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	struct riot_bin bin = {0};

	struct riot_bin_stream _stream = {
		.buf = {
			.ptr = buf,
			.len = len,
		},
		.cur = 0,
	}, *stream = &_stream;

	/* calculate the allocation requirements in terms of number of nodes,
	 * fields, and pairs so that all required memory can be allocated
	 * ahead of time in 3 separate blocks. this means that all memory can
	 * be easily deallocated when it is finished with and the parser does
	 * not have to worry about allocation potentially failing halfway
	 * through and having to step back and undo all previous allocations
	 */
	err = riot_bin_size(buf, len, &bin.mempool);
	BIN_ASSERT(!err, err, failure, "Failed to calculate allocation requirements for bin repr!")

	assert(bin.mempool.nodes.len);
	assert(bin.mempool.fields.len);
	assert(bin.mempool.pairs.len);

	dbglog("String Arena: %zu\n", bin.mempool.strings.len);
	dbglog("Arena-allocated Nodes: %zu\n", bin.mempool.nodes.len);
	dbglog("Arena-allocated Fields: %zu\n", bin.mempool.fields.len);
	dbglog("Arena-allocated Pairs: %zu\n", bin.mempool.pairs.len);

	bin.mempool.strings.ptr = malloc(bin.mempool.strings.len * sizeof(char));
	BIN_ASSERT(bin.mempool.strings.ptr, RIOT_IO_ERROR_ALLOC, failure, "Failed to allocate string arena for bin repr!")

	bin.mempool.nodes.ptr = malloc(bin.mempool.nodes.len * sizeof(struct riot_bin_node));
	BIN_ASSERT(bin.mempool.nodes.ptr, RIOT_IO_ERROR_ALLOC, failure, "Failed to allocate nodes for bin repr!")

	bin.mempool.fields.ptr = malloc(bin.mempool.fields.len * sizeof(struct riot_bin_field));
	BIN_ASSERT(bin.mempool.fields.ptr, RIOT_IO_ERROR_ALLOC, failure, "Failed to allocate fields for bin repr!")

	bin.mempool.pairs.ptr = malloc(bin.mempool.pairs.len * sizeof(struct riot_bin_pair));
	BIN_ASSERT(bin.mempool.pairs.ptr, RIOT_IO_ERROR_ALLOC, failure, "Failed to allocate pairs for bin repr!")

	bin.mempool.strings.head = bin.mempool.strings.ptr;
	bin.mempool.nodes.head = bin.mempool.nodes.ptr;
	bin.mempool.fields.head = bin.mempool.fields.ptr;
	bin.mempool.pairs.head = bin.mempool.pairs.ptr;

	/* the initial header of the inibin file depends on a magic identifier.
	 * if this identifier is equal to PTCH, then an 8-byte unknown field is
	 * present and the inibin contains a trailing list of patches which
	 * must be read. after reading the PTCH header (or if it was not
	 * present) the PROP magic identifier must be present, otherwise the
	 * file is in an unknown format or corrupted
	 */
	u8 magic[4] = {0}, patch_magic[4] = "PTCH", prop_magic[4] = "PROP";
	bool has_patches = false;

	bin.type_section.type = RIOT_BIN_NODE_TYPE_STR;

	err = riot_bin_stream_consume(stream, magic, sizeof magic);
	BIN_ASSERT(!err, err, failure, "Failed to read magic value!")

	if (memcmp(magic, patch_magic, sizeof magic) == 0) {
		dbglog("Read PTCH magic (have patch header and trailer)\n");

		bin.type_section.node_str.ptr = "PTCH";
		bin.type_section.node_str.len = 4;

		has_patches = true;

		u64 unknown = 0;
		err = riot_bin_stream_consume(stream, &unknown, sizeof(unknown));
		BIN_ASSERT(!err, err, failure, "Failed to read unknown bytes!")

		dbglog("Unknown bytes: %llu\n", unknown);

		err = riot_bin_stream_consume(stream, magic, sizeof magic);
		BIN_ASSERT(!err, err, failure, "Failed to read magic value!")
	}

	/* if we do not have the PROP magic identifier, our file is corrupt
	 * or has an unknown format
	 */
	if (memcmp(magic, prop_magic, sizeof magic) != 0) {
		/* if we don't successfully read the patch magic identifier or
		 * the prop magic identifier then we have a corrupted bin file
		 */
		BIN_ASSERT(false, RIOT_IO_ERROR_CORRUPT, failure, "Invalid magic value read!");
	}

	bin.type_section.node_str.ptr = "PROP";
	bin.type_section.node_str.len = 4;

	/* we need to parse the version identifier of the current file so that
	 * we can correctly parse out the included sections
	 */
	u32 version = 0;
	err = riot_bin_stream_consume(stream, &version, sizeof(version));
	BIN_ASSERT(!err, err, failure, "Failed to read INIBIN version!")

	dbglog("INIBIN version: %u\n", version);
	assert(version > 0);

	bin.version_section.type = RIOT_BIN_NODE_TYPE_U32;
	bin.version_section.node_u32 = version;

	if (version >= 2) {
		err = riot_bin_read_linked_files(stream, &bin);
		BIN_ASSERT(!err, err, failure, "Failed to read linked files!")
	} else {
		bin.linked_section.type = RIOT_BIN_NODE_TYPE_LIST;
		bin.linked_section.node_list.ty = RIOT_BIN_NODE_TYPE_NONE;
		bin.linked_section.node_list.count = 0;
		bin.linked_section.node_list.vs = NULL;
	}

	err = riot_bin_read_entries(stream, &bin);
	BIN_ASSERT(!err, err, failure, "Failed to read prop entries!")

	if (version >= 3 && has_patches) {
		err = riot_bin_read_patches(stream, &bin);
		BIN_ASSERT(!err, err, failure, "Failed to read patch entries!");
	} else {
		bin.patches_section.type = RIOT_BIN_NODE_TYPE_MAP;
		bin.patches_section.node_map.key_ty = RIOT_BIN_NODE_TYPE_NONE;
		bin.patches_section.node_map.val_ty = RIOT_BIN_NODE_TYPE_NONE;
		bin.patches_section.node_map.count = 0;
		bin.patches_section.node_map.vs = NULL;
	}

	BIN_ASSERT(riot_bin_stream_eof(stream), RIOT_IO_ERROR_CORRUPT, failure, "Stream input remaining!")

	*out = bin;

	return err;

failure:
	riot_bin_free(&bin);

	return err;
}

static enum riot_io_error
riot_bin_read_linked_files(struct riot_bin_stream *stream, struct riot_bin *bin) {
	assert(stream);
	assert(bin);

	dbglog("Reading linked files\n");

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	struct riot_bin_node list = {
		.type = RIOT_BIN_NODE_TYPE_LIST,
		.node_list = {
			.ty = RIOT_BIN_NODE_TYPE_STR,
		},
	};

	err = riot_bin_stream_consume(stream, &list.node_list.count, sizeof(list.node_list.count));
	BIN_ASSERT(!err, err, failure, "Failed to read number of linked files!")

	dbglog("Linked files: %u\n", list.node_list.count);

	if (!list.node_list.count)
		goto success;

	list.node_list.vs = bin->mempool.nodes.head;
	bin->mempool.nodes.head += list.node_list.count;

	for (u32 i = 0; i < list.node_list.count; i++) {
		struct riot_bin_node *elem = &list.node_list.vs[i];

		err = riot_bin_read_node(stream, RIOT_BIN_NODE_TYPE_STR, elem, &bin->mempool);
		BIN_ASSERT(!err, err, failure, "Failed to read linked files %u!", i)
	}

success:
	bin->linked_section = list;

failure:
	return err;
}

static enum riot_io_error
riot_bin_read_entry(struct riot_bin_stream *stream, hashes_fnv1a_val_t *name_hash,
			struct riot_bin_field_list *embed, struct riot_bin_mempool *mempool) {
	assert(stream);
	assert(name_hash);
	assert(embed);
	assert(mempool);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	u32 size = 0;
	err = riot_bin_stream_consume(stream, &size, sizeof(size));
	BIN_ASSERT(!err, err, failure, "Failed to read total size of prop entry embed!")

	size_t prev_cur = stream->cur;

	err = riot_bin_stream_consume(stream, name_hash, sizeof(*name_hash));
	BIN_ASSERT(!err, err, failure, "Failed to read prop entry embed name hash!")

	err = riot_bin_stream_consume(stream, &embed->count, sizeof(embed->count));
	BIN_ASSERT(!err, err, failure, "Failed to read count of prop entry embed fields!")

	embed->vs = mempool->fields.head;
	mempool->fields.head += embed->count;

	for (u16 i = 0; i < embed->count; i++) {
		struct riot_bin_field *elem = &embed->vs[i];

		err = riot_bin_stream_consume(stream, &elem->hash, sizeof(elem->hash));
		BIN_ASSERT(!err, err, failure, "Failed to read prop entry embed's field %u name hash!", i)

		u8 raw_type;
		err = riot_bin_stream_consume(stream, &raw_type, sizeof(raw_type));
		BIN_ASSERT(!err, err, failure, "Failed to read prop entry embed's field %u type!", i)

		enum riot_bin_node_type type = riot_bin_node_type_from_raw(raw_type);
		err = riot_bin_read_node(stream, type, &elem->val, mempool);
		BIN_ASSERT(!err, err, failure, "Failed to read prop entry embed's field %u value!", i)
	}

	BIN_ASSERT(stream->cur - prev_cur == size, RIOT_IO_ERROR_CORRUPT, failure,
			"Prop entry embed parsing failed (%zu bytes read, %u bytes expected)!", stream->cur - prev_cur, size)

failure:
	return err;
}

static enum riot_io_error
riot_bin_read_entries(struct riot_bin_stream *stream, struct riot_bin *bin) {
	assert(stream);
	assert(bin);

	dbglog("Reading prop entries\n");

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	struct riot_bin_node map = {
		.type = RIOT_BIN_NODE_TYPE_MAP,
		.node_map = {
			.key_ty = RIOT_BIN_NODE_TYPE_HASH,
			.val_ty = RIOT_BIN_NODE_TYPE_EMBED,
		},
	};

	err = riot_bin_stream_consume(stream, &map.node_map.count, sizeof(map.node_map.count));
	BIN_ASSERT(!err, err, failure, "Failed to read number of prop entries!")

	dbglog("Prop entries: %u\n", map.node_map.count);

	if (!map.node_map.count) goto success;

	hashes_fnv1a_val_t *entry_name_hashes = (hashes_fnv1a_val_t*)&stream->buf.ptr[stream->cur];
	err = riot_bin_stream_skip(stream, map.node_map.count * sizeof(hashes_fnv1a_val_t));
	BIN_ASSERT(!err, err, failure, "Failed to skip over prop entry name hashes!")

	map.node_map.vs = bin->mempool.pairs.head;
	bin->mempool.pairs.head += map.node_map.count;

	for (u32 i = 0; i < map.node_map.count; i++) {
		hashes_fnv1a_val_t entry_name_hash = entry_name_hashes[i];

		struct riot_bin_pair *elem = &map.node_map.vs[i];
		elem->key.type = RIOT_BIN_NODE_TYPE_HASH;
		elem->val.type = RIOT_BIN_NODE_TYPE_EMBED;
		elem->val.node_embed.hash = entry_name_hash;

		err = riot_bin_read_entry(stream, &elem->key.node_hash, &elem->val.node_embed, &bin->mempool);
		BIN_ASSERT(!err, err, failure, "Failed to read embed entry %u!", i)
	}

success:
	bin->entries_section = map;

failure:
	return err;
}

static enum riot_io_error
riot_bin_read_patch(struct riot_bin_stream *stream, hashes_fnv1a_val_t *name_hash,
			struct riot_bin_field_list *embed, struct riot_bin_mempool *mempool) {
	assert(stream);
	assert(name_hash);
	assert(embed);
	assert(mempool);

	hashes_fnv1a_val_t path_name_hash = hashes_fnv1a("path", 4, HASHES_FNV1A_DEFAULT_SEED);
	hashes_fnv1a_val_t value_name_hash = hashes_fnv1a("value", 5, HASHES_FNV1A_DEFAULT_SEED);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	err = riot_bin_stream_consume(stream, name_hash, sizeof(*name_hash));
	BIN_ASSERT(!err, err, failure, "Failed to read patch entry embed name hash!")

	u32 size = 0;
	err = riot_bin_stream_consume(stream, &size, sizeof(size));
	BIN_ASSERT(!err, err, failure, "Failed to read total size of patch entry embed!")

	size_t prev_cur = stream->cur;

	embed->vs = mempool->fields.head;
	mempool->fields.head += 2;

	embed->vs[0].hash = path_name_hash;
	struct riot_bin_node *path = &embed->vs[0].val;

	embed->vs[1].hash = value_name_hash;
	struct riot_bin_node *value = &embed->vs[1].val;

	u8 raw_type;
	err = riot_bin_stream_consume(stream, &raw_type, sizeof(raw_type));
	BIN_ASSERT(!err, err, failure, "Failed to read raw type of patch entry embed value!");

	enum riot_bin_node_type type = riot_bin_node_type_from_raw(raw_type);

	err = riot_bin_read_node(stream, RIOT_BIN_NODE_TYPE_STR, path, mempool);
	BIN_ASSERT(!err, err, failure, "Failed to read patch entry embed path!")

	err = riot_bin_read_node(stream, type, value, mempool);
	BIN_ASSERT(!err, err, failure, "Failed to read patch entry embed value!")

	BIN_ASSERT(stream->cur - prev_cur == size, RIOT_IO_ERROR_ALLOC, failure,
			"Patch entry embed parsing failed (%zu bytes read, %u bytes expected)!", stream->cur - prev_cur, size)

failure:
	return err;
}

static enum riot_io_error
riot_bin_read_patches(struct riot_bin_stream *stream, struct riot_bin *bin) {
	assert(stream);
	assert(bin);

	dbglog("Reading prop patches\n");

	hashes_fnv1a_val_t patch_name_hash = hashes_fnv1a("patch", 5, HASHES_FNV1A_DEFAULT_SEED);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	struct riot_bin_node map = {
		.type = RIOT_BIN_NODE_TYPE_MAP,
		.node_map = {
			.key_ty = RIOT_BIN_NODE_TYPE_HASH,
			.val_ty = RIOT_BIN_NODE_TYPE_EMBED,
		},
	};

	err = riot_bin_stream_consume(stream, &map.node_map.count, sizeof(map.node_map.count));
	BIN_ASSERT(!err, err, failure, "Failed to read number of patches!")

	if (!map.node_map.count) goto success;

	map.node_map.vs = bin->mempool.pairs.head;
	bin->mempool.pairs.head += map.node_map.count;

	for (u32 i = 0; i < map.node_map.count; i++) {
		struct riot_bin_pair *elem = &map.node_map.vs[i];
		elem->key.type = RIOT_BIN_NODE_TYPE_HASH;
		elem->val.type = RIOT_BIN_NODE_TYPE_EMBED;
		elem->val.node_embed.hash = patch_name_hash;

		err = riot_bin_read_patch(stream, &elem->key.node_hash, &elem->val.node_embed, &bin->mempool);
		BIN_ASSERT(!err, err, failure, "Failed to read patch %u!", i)
	}

success:
	bin->patches_section = map;

failure:
	return err;
}

/* ===========================================================================
 * Sizing Helpers
 * ===========================================================================
 */
static enum riot_io_error
riot_bin_size_str(struct riot_bin_stream *stream, struct riot_bin_mempool *mempool) {
	assert(stream);
	assert(mempool);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	u16 len = 0;
	err = riot_bin_stream_consume(stream, &len, sizeof(len));
	BIN_ASSERT(!err, err, failure, "Failed to read string length!")

	err = riot_bin_stream_skip(stream, len);
	BIN_ASSERT(!err, err, failure, "Failed to skip string bytes!")

	mempool->strings.len += len;

failure:
	return err;
}

static enum riot_io_error
riot_bin_size_ptr(struct riot_bin_stream *stream, struct riot_bin_mempool *mempool) {
	assert(stream);
	assert(mempool);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	hashes_fnv1a_val_t name_hash = 0;
	err = riot_bin_stream_consume(stream, &name_hash, sizeof(name_hash));
	BIN_ASSERT(!err, err, failure, "Failed to read ptr name hash!")

	/* if the hash is empty, we have a null pointer and so we can skip this
	 * node (err == RIOT_IO_ERROR_OK)
	 */
	if (name_hash == 0)
		return err;

	u32 size = 0;
	err = riot_bin_stream_consume(stream, &size, sizeof(size));
	BIN_ASSERT(!err, err, failure, "Failed to read the total size")

	size_t prev_cur = stream->cur;

	u16 count = 0;
	err = riot_bin_stream_consume(stream, &count, sizeof(count));
	BIN_ASSERT(!err, err, failure, "Failed to read number of elements after pointer!")

	for (u16 i = 0; i < count; i++) {
		err = riot_bin_stream_skip(stream, sizeof(hashes_fnv1a_val_t));
		BIN_ASSERT(!err, err, failure, "Failed to skip ptr element %u name hash!", i)

		u8 raw_type;
		err = riot_bin_stream_consume(stream, &raw_type, sizeof(raw_type));
		BIN_ASSERT(!err, err, failure, "Failed to read ptr element %u type!", i)

		enum riot_bin_node_type type = riot_bin_node_type_from_raw(raw_type);
		err = riot_bin_size_node(stream, type, mempool);
		BIN_ASSERT(!err, err, failure, "Failed to size ptr element %u!", i)
	}

	BIN_ASSERT(stream->cur - prev_cur == size, RIOT_IO_ERROR_CORRUPT, failure,
			"Ptr parsing failed (%zu bytes read, %u bytes expected)!", stream->cur - prev_cur, size)

	mempool->fields.len += count;

failure:
	return err;
}

static enum riot_io_error
riot_bin_size_embed(struct riot_bin_stream *stream, struct riot_bin_mempool *mempool) {
	assert(stream);
	assert(mempool);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	err = riot_bin_stream_skip(stream, sizeof(hashes_fnv1a_val_t));
	BIN_ASSERT(!err, err, failure, "Failed to skip embed name hash!")

	u32 size = 0;
	err = riot_bin_stream_consume(stream, &size, sizeof(size));
	BIN_ASSERT(!err, err, failure, "Failed to read total size of embed!")

	size_t prev_cur = stream->cur;

	u16 count = 0;
	err = riot_bin_stream_consume(stream, &count, sizeof(count));
	BIN_ASSERT(!err, err, failure, "Failed to read number of elements in embed!")

	for (u16 i = 0; i < count; i++) {
		err = riot_bin_stream_skip(stream, sizeof(hashes_fnv1a_val_t));
		BIN_ASSERT(!err, err, failure, "Failed to skip embed element %u name hash!", i)

		u8 raw_type;
		err = riot_bin_stream_consume(stream, &raw_type, sizeof(raw_type));
		BIN_ASSERT(!err, err, failure, "Failed to read embed element %u raw type!", i)

		enum riot_bin_node_type type = riot_bin_node_type_from_raw(raw_type);
		err = riot_bin_size_node(stream, type, mempool);
		BIN_ASSERT(!err, err, failure, "Failed to size embed element %u!", i)
	}

	BIN_ASSERT(stream->cur - prev_cur == size, RIOT_IO_ERROR_CORRUPT, failure,
			"Embed parsing failed (%zu bytes read, %u bytes expected)!", stream->cur - prev_cur, size)

	mempool->fields.len += count;

failure:
	return err;
}

static enum riot_io_error
riot_bin_size_list(struct riot_bin_stream *stream, struct riot_bin_mempool *mempool) {
	assert(stream);
	assert(mempool);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	u8 raw_type = RIOT_BIN_NODE_TYPE_NONE;
	err = riot_bin_stream_consume(stream, &raw_type, sizeof(raw_type));
	BIN_ASSERT(!err, err, failure, "Failed to read list element type!")

	enum riot_bin_node_type type = riot_bin_node_type_from_raw(raw_type);
	BIN_ASSERT(!riot_bin_node_type_is_container(type), RIOT_IO_ERROR_CORRUPT, failure, "List cannot contain other container types")

	u32 size = 0;
	err = riot_bin_stream_consume(stream, &size, sizeof(size));
	BIN_ASSERT(!err, err, failure, "Failed to read total size of list!")

	size_t prev_cur = stream->cur;

	u32 count = 0;
	err = riot_bin_stream_consume(stream, &count, sizeof(count));
	BIN_ASSERT(!err, err, failure, "Failed to read number of elements in list!")

	for (u32 i = 0; i < count; i++) {
		err = riot_bin_size_node(stream, type, mempool);
		BIN_ASSERT(!err, err, failure, "Failed to read list element %u", i)
	}

	BIN_ASSERT(stream->cur - prev_cur == size, RIOT_IO_ERROR_CORRUPT, failure,
			"List parsing failed (%zu bytes read, %u bytes expected)!", stream->cur - prev_cur, size)

failure:
	return err;
}

static enum riot_io_error
riot_bin_size_option(struct riot_bin_stream *stream, struct riot_bin_mempool *mempool) {
	assert(stream);
	assert(mempool);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	u8 raw_type = RIOT_BIN_NODE_TYPE_NONE;
	err = riot_bin_stream_consume(stream, &raw_type, sizeof(raw_type));
	BIN_ASSERT(!err, err, failure, "Failed to read option item type!")

	enum riot_bin_node_type type = riot_bin_node_type_from_raw(raw_type);
	BIN_ASSERT(!riot_bin_node_type_is_container(type), RIOT_IO_ERROR_CORRUPT, failure, "Option cannot contain other container types!")

	u8 has_item = 0;
	err = riot_bin_stream_consume(stream, &has_item, sizeof(has_item));
	BIN_ASSERT(!err, err, failure, "Failed to read option item presence flag!")

	/* if the option has no value, we can simply skip it
	 */
	if (!has_item) return err;

	err = riot_bin_size_node(stream, type, mempool);
	BIN_ASSERT(!err, err, failure, "Failed to size option item!")

failure:
	return err;
}

static enum riot_io_error
riot_bin_size_map(struct riot_bin_stream *stream, struct riot_bin_mempool *mempool) {
	assert(stream);
	assert(mempool);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	u8 raw_key_ty = RIOT_BIN_NODE_TYPE_NONE, raw_val_ty = RIOT_BIN_NODE_TYPE_NONE;
	err = riot_bin_stream_consume(stream, &raw_key_ty, sizeof(raw_key_ty));
	BIN_ASSERT(!err, err, failure, "Failed to read map key type!")

	err = riot_bin_stream_consume(stream, &raw_val_ty, sizeof(raw_val_ty));
	BIN_ASSERT(!err, err, failure, "Failed to read map val type!")

	enum riot_bin_node_type key_ty = riot_bin_node_type_from_raw(raw_key_ty);
	BIN_ASSERT(riot_bin_node_type_is_primitive(key_ty), RIOT_IO_ERROR_CORRUPT, failure, "Map keys must be primitive!")

	enum riot_bin_node_type val_ty = riot_bin_node_type_from_raw(raw_val_ty);
	BIN_ASSERT(!riot_bin_node_type_is_container(val_ty), RIOT_IO_ERROR_CORRUPT, failure, "Map vals must not be containers!")

	u32 size = 0;
	err = riot_bin_stream_consume(stream, &size, sizeof(size));
	BIN_ASSERT(!err, err, failure, "Failed to read map total size!")

	size_t prev_cur = stream->cur;

	u32 count = 0;
	err = riot_bin_stream_consume(stream, &count, sizeof(count));
	BIN_ASSERT(!err, err, failure, "Failed to read number of elements in map!")

	for (u32 i = 0; i < count; i++) {
		err = riot_bin_size_node(stream, key_ty, mempool);
		BIN_ASSERT(!err, err, failure, "Failed to size map key %u", i)

		err = riot_bin_size_node(stream, val_ty, mempool);
		BIN_ASSERT(!err, err, failure, "Failed to size map val %u", i)
	}

	BIN_ASSERT(stream->cur - prev_cur == size, RIOT_IO_ERROR_CORRUPT, failure,
			"Map parsing failed (%zu bytes read, %u bytes expected)!", stream->cur - prev_cur, size)

	mempool->pairs.len += count;

failure:
	return err;
}

static enum riot_io_error
riot_bin_size_node(struct riot_bin_stream *stream, enum riot_bin_node_type type, struct riot_bin_mempool *mempool) {
	assert(stream);
	assert(mempool);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	switch (type) {
		/* handle arithmetic types with a known size by simply reading
		 * however many bytes are required into the union data. this
		 * assumes that the union elements all alias to the first
		 * memory location
		 */
		case RIOT_BIN_NODE_TYPE_B8:
		case RIOT_BIN_NODE_TYPE_I8:
		case RIOT_BIN_NODE_TYPE_U8:
		case RIOT_BIN_NODE_TYPE_I16:
		case RIOT_BIN_NODE_TYPE_U16:
		case RIOT_BIN_NODE_TYPE_I32:
		case RIOT_BIN_NODE_TYPE_U32:
		case RIOT_BIN_NODE_TYPE_I64:
		case RIOT_BIN_NODE_TYPE_U64:
		case RIOT_BIN_NODE_TYPE_F32:
		case RIOT_BIN_NODE_TYPE_VEC2:
		case RIOT_BIN_NODE_TYPE_VEC3:
		case RIOT_BIN_NODE_TYPE_VEC4:
		case RIOT_BIN_NODE_TYPE_MAT4:
		case RIOT_BIN_NODE_TYPE_RGBA:
		case RIOT_BIN_NODE_TYPE_HASH:
		case RIOT_BIN_NODE_TYPE_FILE:
		case RIOT_BIN_NODE_TYPE_LINK:
		case RIOT_BIN_NODE_TYPE_FLAG:
			err = riot_bin_stream_skip(stream, riot_bin_node_type_to_size(type));
			BIN_ASSERT(!err, err, failure, "Failed to skip bytes for primitive node of type %u!", type)
			break;

		/* complex types require special-case handling, as they usually
		 * handle some kind of memory allocation which can fail
		 */
		case RIOT_BIN_NODE_TYPE_STR:
			err = riot_bin_size_str(stream, mempool);
			BIN_ASSERT(!err, err, failure, "Failed to size string node!")
			break;

		case RIOT_BIN_NODE_TYPE_LIST:
		case RIOT_BIN_NODE_TYPE_LIST2:
			err = riot_bin_size_list(stream, mempool);
			BIN_ASSERT(!err, err, failure, "Failed to size list node!")
			break;

		case RIOT_BIN_NODE_TYPE_PTR:
			err = riot_bin_size_ptr(stream, mempool);
			BIN_ASSERT(!err, err, failure, "Failed to size ptr node!")
			break;

		case RIOT_BIN_NODE_TYPE_EMBED:
			err = riot_bin_size_embed(stream, mempool);
			BIN_ASSERT(!err, err, failure, "Failed to size embed node!")
			break;

		case RIOT_BIN_NODE_TYPE_OPTION:
			err = riot_bin_size_option(stream, mempool);
			BIN_ASSERT(!err, err, failure, "Failed to size option node!")
			break;

		case RIOT_BIN_NODE_TYPE_MAP:
			err = riot_bin_size_map(stream, mempool);
			BIN_ASSERT(!err, err, failure, "Failed to size map node!")
			break;

		/* an unknown type was encountered, which should never happen
		 */
		default:
			BIN_ASSERT(false, RIOT_IO_ERROR_CORRUPT, failure, "Unknown node type %u encountered!", type)
			break;
	}

	mempool->nodes.len++;

failure:
	return err;
}

/* ===========================================================================
 * Reading Helpers
 * ===========================================================================
 */
static enum riot_io_error
riot_bin_read_str(struct riot_bin_stream *stream, struct riot_bin_node *out, struct riot_bin_mempool *mempool) {
	assert(stream);
	assert(out);
	assert(mempool);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	err = riot_bin_stream_consume(stream, &out->node_str.len, sizeof(out->node_str.len));
	BIN_ASSERT(!err, err, failure, "Failed to read string length!")

	out->node_str.ptr = mempool->strings.head;
	mempool->strings.head += out->node_str.len;

	err = riot_bin_stream_consume(stream, out->node_str.ptr, out->node_str.len);
	BIN_ASSERT(!err, err, failure, "Failed to skip string bytes!")

failure:
	return err;
}

static enum riot_io_error
riot_bin_read_ptr(struct riot_bin_stream *stream, struct riot_bin_node *out, struct riot_bin_mempool *mempool) {
	assert(stream);
	assert(out);
	assert(mempool);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	err = riot_bin_stream_consume(stream, &out->node_ptr.hash, sizeof(out->node_ptr.hash));
	BIN_ASSERT(!err, err, failure, "Failed to read ptr name hash!")

	/* if the hash is empty, we have a null pointer and so we can skip this
	 * node (err == RIOT_IO_ERROR_OK)
	 */
	if (out->node_ptr.hash == 0)
		return err;

	u32 size = 0;
	err = riot_bin_stream_consume(stream, &size, sizeof(size));
	BIN_ASSERT(!err, err, failure, "Failed to read the total size")

	size_t prev_cur = stream->cur;

	err = riot_bin_stream_consume(stream, &out->node_ptr.count, sizeof(out->node_ptr.count));
	BIN_ASSERT(!err, err, failure, "Failed to read number of elements after pointer!")

	out->node_ptr.vs = mempool->fields.head;
	mempool->fields.head += out->node_ptr.count;

	for (u16 i = 0; i < out->node_ptr.count; i++) {
		struct riot_bin_field *elem = &out->node_ptr.vs[i];

		err = riot_bin_stream_consume(stream, &elem->hash, sizeof(elem->hash));
		BIN_ASSERT(!err, err, failure, "Failed to read ptr element %u name hash!", i)

		u8 raw_type;
		err = riot_bin_stream_consume(stream, &raw_type, sizeof(raw_type));
		BIN_ASSERT(!err, err, failure, "Failed to read ptr element %u type!", i)

		enum riot_bin_node_type type = riot_bin_node_type_from_raw(raw_type);
		err = riot_bin_read_node(stream, type, &elem->val, mempool);
		BIN_ASSERT(!err, err, failure, "Failed to read ptr element %u!", i)
	}

	BIN_ASSERT(stream->cur - prev_cur == size, RIOT_IO_ERROR_CORRUPT, failure,
			"Ptr parsing failed (%zu bytes read, %u bytes expected)!", stream->cur - prev_cur, size)

failure:
	return err;
}

static enum riot_io_error
riot_bin_read_embed(struct riot_bin_stream *stream, struct riot_bin_node *out, struct riot_bin_mempool *mempool) {
	assert(stream);
	assert(out);
	assert(mempool);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	err = riot_bin_stream_consume(stream, &out->node_embed.hash, sizeof(out->node_embed.hash));
	BIN_ASSERT(!err, err, failure, "Failed to read embed name hash!")

	u32 size = 0;
	err = riot_bin_stream_consume(stream, &size, sizeof(size));
	BIN_ASSERT(!err, err, failure, "Failed to read total size of embed!")

	size_t prev_cur = stream->cur;

	err = riot_bin_stream_consume(stream, &out->node_embed.count, sizeof(out->node_embed.count));
	BIN_ASSERT(!err, err, failure, "Failed to read number of elements in embed!")

	out->node_embed.vs = mempool->fields.head;
	mempool->fields.head += out->node_embed.count;

	for (u16 i = 0; i < out->node_embed.count; i++) {
		struct riot_bin_field *elem = &out->node_embed.vs[i];

		err = riot_bin_stream_consume(stream, &elem->hash, sizeof(elem->hash));
		BIN_ASSERT(!err, err, failure, "Failed to read embed element %u name hash!", i)

		u8 raw_type;
		err = riot_bin_stream_consume(stream, &raw_type, sizeof(raw_type));
		BIN_ASSERT(!err, err, failure, "Failed to read embed element %u raw type!", i)

		enum riot_bin_node_type type = riot_bin_node_type_from_raw(raw_type);
		err = riot_bin_read_node(stream, type, &elem->val, mempool);
		BIN_ASSERT(!err, err, failure, "Failed to read embed element %u!", i)
	}

	BIN_ASSERT(stream->cur - prev_cur == size, RIOT_IO_ERROR_CORRUPT, failure,
			"Embed parsing failed (%zu bytes read, %u bytes expected)!", stream->cur - prev_cur, size)

failure:
	return err;
}


static enum riot_io_error
riot_bin_read_list(struct riot_bin_stream *stream, struct riot_bin_node *out, struct riot_bin_mempool *mempool) {
	assert(stream);
	assert(out);
	assert(mempool);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	u8 raw_type = RIOT_BIN_NODE_TYPE_NONE;
	err = riot_bin_stream_consume(stream, &raw_type, sizeof(raw_type));
	BIN_ASSERT(!err, err, failure, "Failed to read list element type!")

	out->node_list.ty = riot_bin_node_type_from_raw(raw_type);
	BIN_ASSERT(!riot_bin_node_type_is_container(out->node_list.ty), RIOT_IO_ERROR_CORRUPT, failure, "List cannot contain other container types")

	u32 size = 0;
	err = riot_bin_stream_consume(stream, &size, sizeof(size));
	BIN_ASSERT(!err, err, failure, "Failed to read total size of list!")

	size_t prev_cur = stream->cur;

	err = riot_bin_stream_consume(stream, &out->node_list.count, sizeof(out->node_list.count));
	BIN_ASSERT(!err, err, failure, "Failed to read number of elements in list!")

	out->node_list.vs = mempool->nodes.head;
	mempool->nodes.head += out->node_list.count;

	for (u32 i = 0; i < out->node_list.count; i++) {
		struct riot_bin_node *elem = &out->node_list.vs[i];

		err = riot_bin_read_node(stream, out->node_list.ty, elem, mempool);
		BIN_ASSERT(!err, err, failure, "Failed to read list element %u", i)
	}

	BIN_ASSERT(stream->cur - prev_cur == size, RIOT_IO_ERROR_CORRUPT, failure,
			"List parsing failed (%zu bytes read, %u bytes expected)!", stream->cur - prev_cur, size)

failure:
	return err;
}

static enum riot_io_error
riot_bin_read_option(struct riot_bin_stream *stream, struct riot_bin_node *out, struct riot_bin_mempool *mempool) {
	assert(stream);
	assert(out);
	assert(mempool);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	u8 raw_type = RIOT_BIN_NODE_TYPE_NONE;
	err = riot_bin_stream_consume(stream, &raw_type, sizeof(raw_type));
	BIN_ASSERT(!err, err, failure, "Failed to read option item type!")

	out->node_option.ty = riot_bin_node_type_from_raw(raw_type);
	BIN_ASSERT(!riot_bin_node_type_is_container(out->node_option.ty), RIOT_IO_ERROR_CORRUPT, failure, "Option cannot contain other container types!")

	u8 has_item = 0;
	err = riot_bin_stream_consume(stream, &has_item, sizeof(has_item));
	BIN_ASSERT(!err, err, failure, "Failed to read option item presence flag!")

	/* if the option has no value, we can simply skip it
	 */
	if (!has_item) return err;

	out->node_option.vp = mempool->nodes.head;
	mempool->nodes.head += 1;

	err = riot_bin_read_node(stream, out->node_option.ty, out->node_option.vp, mempool);
	BIN_ASSERT(!err, err, failure, "Failed to read option item!")

failure:
	return err;
}

static enum riot_io_error
riot_bin_read_map(struct riot_bin_stream *stream, struct riot_bin_node *out, struct riot_bin_mempool *mempool) {
	assert(stream);
	assert(out);
	assert(mempool);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	u8 raw_key_ty = RIOT_BIN_NODE_TYPE_NONE, raw_val_ty = RIOT_BIN_NODE_TYPE_NONE;
	err = riot_bin_stream_consume(stream, &raw_key_ty, sizeof(raw_key_ty));
	BIN_ASSERT(!err, err, failure, "Failed to read map key type!")

	err = riot_bin_stream_consume(stream, &raw_val_ty, sizeof(raw_val_ty));
	BIN_ASSERT(!err, err, failure, "Failed to read map val type!")

	out->node_map.key_ty = riot_bin_node_type_from_raw(raw_key_ty);
	BIN_ASSERT(riot_bin_node_type_is_primitive(out->node_map.key_ty), RIOT_IO_ERROR_CORRUPT, failure, "Map keys must be primitive!")

	out->node_map.val_ty = riot_bin_node_type_from_raw(raw_val_ty);
	BIN_ASSERT(!riot_bin_node_type_is_container(out->node_map.val_ty), RIOT_IO_ERROR_CORRUPT, failure, "Map vals must not be containers!")

	u32 size = 0;
	err = riot_bin_stream_consume(stream, &size, sizeof(size));
	BIN_ASSERT(!err, err, failure, "Failed to read map total size!")

	size_t prev_cur = stream->cur;

	err = riot_bin_stream_consume(stream, &out->node_map.count, sizeof(out->node_map.count));
	BIN_ASSERT(!err, err, failure, "Failed to read number of elements in map!")

	out->node_map.vs = mempool->pairs.head;
	mempool->pairs.head += out->node_map.count;

	for (u32 i = 0; i < out->node_map.count; i++) {
		struct riot_bin_pair *elem = &out->node_map.vs[i];

		err = riot_bin_read_node(stream, out->node_map.key_ty, &elem->key, mempool);
		BIN_ASSERT(!err, err, failure, "Failed to read map key %u", i)

		err = riot_bin_read_node(stream, out->node_map.val_ty, &elem->val, mempool);
		BIN_ASSERT(!err, err, failure, "Failed to read map val %u", i)
	}

	BIN_ASSERT(stream->cur - prev_cur == size, RIOT_IO_ERROR_CORRUPT, failure,
			"Map parsing failed (%zu bytes read, %u bytes expected)!", stream->cur - prev_cur, size)

failure:
	return err;
}

static enum riot_io_error
riot_bin_read_node(struct riot_bin_stream *stream, enum riot_bin_node_type type, struct riot_bin_node *out, struct riot_bin_mempool *mempool) {
	assert(stream);
	assert(out);
	assert(mempool);

	enum riot_io_error err = RIOT_IO_ERROR_OK;

	out->type = type;

	switch (type) {
		/* handle arithmetic types with a known size by simply reading
		 * however many bytes are required into the union data. this
		 * assumes that the union elements all alias to the first
		 * memory location
		 */
		case RIOT_BIN_NODE_TYPE_B8:
		case RIOT_BIN_NODE_TYPE_I8:
		case RIOT_BIN_NODE_TYPE_U8:
		case RIOT_BIN_NODE_TYPE_I16:
		case RIOT_BIN_NODE_TYPE_U16:
		case RIOT_BIN_NODE_TYPE_I32:
		case RIOT_BIN_NODE_TYPE_U32:
		case RIOT_BIN_NODE_TYPE_I64:
		case RIOT_BIN_NODE_TYPE_U64:
		case RIOT_BIN_NODE_TYPE_F32:
		case RIOT_BIN_NODE_TYPE_VEC2:
		case RIOT_BIN_NODE_TYPE_VEC3:
		case RIOT_BIN_NODE_TYPE_VEC4:
		case RIOT_BIN_NODE_TYPE_MAT4:
		case RIOT_BIN_NODE_TYPE_RGBA:
		case RIOT_BIN_NODE_TYPE_HASH:
		case RIOT_BIN_NODE_TYPE_FILE:
		case RIOT_BIN_NODE_TYPE_LINK:
		case RIOT_BIN_NODE_TYPE_FLAG:
			err = riot_bin_stream_consume(stream, &out->raw_data, riot_bin_node_type_to_size(type));
			BIN_ASSERT(!err, err, failure, "Failed to read primitive node of type %u!", type)
			break;

		/* complex types require special-case handling, as they usually
		 * handle some kind of memory allocation which can fail
		 */
		case RIOT_BIN_NODE_TYPE_STR:
			err = riot_bin_read_str(stream, out, mempool);
			BIN_ASSERT(!err, err, failure, "Failed to read string node!")
			break;

		case RIOT_BIN_NODE_TYPE_LIST:
		case RIOT_BIN_NODE_TYPE_LIST2:
			err = riot_bin_read_list(stream, out, mempool);
			BIN_ASSERT(!err, err, failure, "Failed to read list node!")
			break;

		case RIOT_BIN_NODE_TYPE_PTR:
			err = riot_bin_read_ptr(stream, out, mempool);
			BIN_ASSERT(!err, err, failure, "Failed to read ptr node!")
			break;

		case RIOT_BIN_NODE_TYPE_EMBED:
			err = riot_bin_read_embed(stream, out, mempool);
			BIN_ASSERT(!err, err, failure, "Failed to read embed node!")
			break;

		case RIOT_BIN_NODE_TYPE_OPTION:
			err = riot_bin_read_option(stream, out, mempool);
			BIN_ASSERT(!err, err, failure, "Failed to read option node!")
			break;

		case RIOT_BIN_NODE_TYPE_MAP:
			err = riot_bin_read_map(stream, out, mempool);
			BIN_ASSERT(!err, err, failure, "Failed to read map node!")
			break;

		/* an unknown type was encountered, which should never happen
		 */
		default:
			BIN_ASSERT(false, RIOT_IO_ERROR_CORRUPT, failure, "Unknown node type %u encountered!", type)
			break;
	}

failure:
	return err;
}

