#include "brzeszczot.h"

static inline size_t
try_read_file(char const *filepath, u8 **out) {
	assert(filepath);
	assert(out);

	FILE *f = fopen(filepath, "rb");
	if (!f) {
		errlog("Failed to open file: %s\n", filepath);
		return 0;
	}

	size_t len, read = 0;
	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fseek(f, 0, SEEK_SET);

	u8 *buf = malloc(len * sizeof(u8));
	if (!buf) {
		errlog("Failed to allocate %zu bytes for file contents buffer\n", len);
		fclose(f);
		return 0;
	}

	do {
		size_t bytes = fread(buf + read, sizeof(u8), len - read, f);
		if (!bytes) {
			errlog("Failed to read bytes from file: %zu/%zu\n", read, len);
			free(buf);
			fclose(f);
			return 0;
		}

		read += bytes;
	} while (read < len);

	fclose(f);

	*out = buf;

	return len;
}

s32
usage(s32 argc, char **argv) {
	(void) argc;

	errlog("Usage: %s <src.bin> <dst.bin>\n", argv[0]);

	return 1;
}

static inline void
__print_tabs(size_t tabs) {
	for (size_t i = 0; i < tabs; i++)
		printf("  ");
}

static inline void
print_riot_bin_node(struct riot_bin_node *val, size_t tab_idx);

static inline void
print_riot_bin_pair(struct riot_bin_pair *val, size_t tab_idx) {
	assert(val);

	print_riot_bin_node(&val->key, tab_idx);
	print_riot_bin_node(&val->val, tab_idx);
}

static inline void
print_riot_bin_field(struct riot_bin_field *val, size_t tab_idx) {
	assert(val);

	__print_tabs(tab_idx);
	printf("field: 0x%0X\n", val->hash);
	print_riot_bin_node(&val->val, tab_idx);
}

static inline void
print_riot_bin_node(struct riot_bin_node *val, size_t tab_idx) {
	assert(val);

	__print_tabs(tab_idx);

	switch (val->type) {
		case RIOT_BIN_NODE_TYPE_B8: {
			printf("b8: %u,\n", val->node_bool);
		} break;

		case RIOT_BIN_NODE_TYPE_I8: {
			printf("i8: %d,\n", val->node_i8);
		} break;

		case RIOT_BIN_NODE_TYPE_U8: {
			printf("u8: %u,\n", val->node_u8);
		} break;

		case RIOT_BIN_NODE_TYPE_I16: {
			printf("i16: %d,\n", val->node_i16);
		} break;

		case RIOT_BIN_NODE_TYPE_U16: {
			printf("u16: %u,\n", val->node_u16);
		} break;

		case RIOT_BIN_NODE_TYPE_I32: {
			printf("i32: %d,\n", val->node_i32);
		} break;

		case RIOT_BIN_NODE_TYPE_U32: {
			printf("u32: %u,\n", val->node_u32);
		} break;

		case RIOT_BIN_NODE_TYPE_I64: {
			printf("i64: %lld,\n", val->node_i64);
		} break;

		case RIOT_BIN_NODE_TYPE_U64: {
			printf("u64: %llu,\n", val->node_u64);
		} break;

		case RIOT_BIN_NODE_TYPE_F32: {
			printf("f32: %f,\n", val->node_f32);
		} break;

		case RIOT_BIN_NODE_TYPE_VEC2: {
			printf("vec2 [ %f, %f ],\n", val->node_vec2.vs[0], val->node_vec2.vs[1]);
		} break;

		case RIOT_BIN_NODE_TYPE_VEC3: {
			printf("vec3 [ %f, %f, %f ],\n", val->node_vec3.vs[0], val->node_vec3.vs[1], val->node_vec3.vs[2]);
		} break;

		case RIOT_BIN_NODE_TYPE_VEC4: {
			printf("vec4 [ %f, %f, %f, %f ],\n", val->node_vec4.vs[0], val->node_vec4.vs[1], val->node_vec4.vs[2], val->node_vec4.vs[3]);
		} break;

		case RIOT_BIN_NODE_TYPE_MAT4: {
			printf("mat4x4 [\n");

			__print_tabs(tab_idx + 1);
			printf("%f, %f, %f, %f\n", val->node_mat4.vs[0], val->node_mat4.vs[1], val->node_mat4.vs[2], val->node_mat4.vs[3]);
			__print_tabs(tab_idx + 1);
			printf("%f, %f, %f, %f\n", val->node_mat4.vs[4], val->node_mat4.vs[5], val->node_mat4.vs[6], val->node_mat4.vs[7]);
			__print_tabs(tab_idx + 1);
			printf("%f, %f, %f, %f\n", val->node_mat4.vs[8], val->node_mat4.vs[9], val->node_mat4.vs[10], val->node_mat4.vs[11]);
			__print_tabs(tab_idx + 1);
			printf("%f, %f, %f, %f\n", val->node_mat4.vs[12], val->node_mat4.vs[13], val->node_mat4.vs[14], val->node_mat4.vs[15]);

			__print_tabs(tab_idx);
			printf("],\n");
		} break;

		case RIOT_BIN_NODE_TYPE_RGBA: {
			printf("rgba: #%0X%0X%0X%0X,\n", val->node_rgba.vs[0], val->node_rgba.vs[1], val->node_rgba.vs[2], val->node_rgba.vs[3]);
		} break;

		case RIOT_BIN_NODE_TYPE_HASH: {
			printf("hash_fnv1a: 0x%0X,\n", val->node_hash);
		} break;

		case RIOT_BIN_NODE_TYPE_FILE: {
			printf("hash_xxh64: 0x%0llX,\n", val->node_file);
		} break;

		case RIOT_BIN_NODE_TYPE_LINK: {
			printf("hash_fnv1a: 0x%0X,\n", val->node_hash);
		} break;

		case RIOT_BIN_NODE_TYPE_FLAG: {
			printf("flag: %u,\n", val->node_flag);
		} break;

		case RIOT_BIN_NODE_TYPE_STR: {
			size_t len = val->node_str.len;
			char *buf = malloc((len + 1) * sizeof(char));
			assert(buf);

			memcpy(buf, val->node_str.ptr, len);
			buf[len] = '\0';

			printf("str: (len %zu) \"%s\",\n", len, buf);

			free(buf);
		} break;

		case RIOT_BIN_NODE_TYPE_LIST:
		case RIOT_BIN_NODE_TYPE_LIST2: {
			printf("list<%s> (len %u) [\n", riot_bin_node_type_str(val->node_list.ty), val->node_list.count);

			for (size_t i = 0; i < val->node_list.count; i++)
				print_riot_bin_node(&val->node_list.vs[i], tab_idx + 1);

			__print_tabs(tab_idx);
			printf("],\n");
		} break;

		case RIOT_BIN_NODE_TYPE_PTR: {
			printf("ptr: hash_fnv1a: 0x%0X (len: %u) [\n", val->node_ptr.hash, val->node_ptr.count);

			for (size_t i = 0; i < val->node_ptr.count; i++)
				print_riot_bin_field(&val->node_ptr.vs[i], tab_idx + 1);

			__print_tabs(tab_idx);
			printf("],\n");
		} break;

		case RIOT_BIN_NODE_TYPE_EMBED: {
			printf("embed: hash_fnv1a: 0x%0X [\n", val->node_embed.hash);

			for (size_t i = 0; i < val->node_embed.count; i++)
				print_riot_bin_field(&val->node_embed.vs[i], tab_idx + 1);

			__print_tabs(tab_idx);
			printf("],\n");
		} break;

		case RIOT_BIN_NODE_TYPE_OPTION: {
			printf("option<%s> [\n", riot_bin_node_type_str(val->node_option.ty));

			if (val->node_option.vp)
				print_riot_bin_node(val->node_option.vp, tab_idx + 1);

			__print_tabs(tab_idx);
			printf("],\n");

		} break;

		case RIOT_BIN_NODE_TYPE_MAP: {
			printf("map<%s, %s> (len: %u) [\n", riot_bin_node_type_str(val->node_map.key_ty),
					riot_bin_node_type_str(val->node_map.val_ty), val->node_map.count);

			for (size_t i = 0; i < val->node_map.count; i++)
				print_riot_bin_pair(&val->node_map.vs[i], tab_idx + 1);

			__print_tabs(tab_idx);
			printf("],\n");
		} break;

		/* an unknown type was encountered, which should never happen
		 */
		default:
			printf("unknown type");
			break;
	}

	fflush(stdout);
}

s32
main(s32 argc, char **argv) {
	if (argc < 3)
		return usage(argc, argv);

	dbglog("Version: " BRZESZCZOT_VERSION "\n");

	u8 *src_buf, *dst_buf;
	size_t src_len = try_read_file(argv[1], &src_buf), dst_len;
	if (!src_len) {
		errlog("Failed to read source file\n");
		return 1;
	}

	enum riot_io_error err;

	struct riot_bin source;
	if ((err = riot_bin_try_read(src_buf, src_len, &source)) != RIOT_IO_ERROR_OK) {
		errlog("Failed to parse source file: %s\n", riot_io_error_str(err));
		return 1;
	}

	errlog("Successfully parsed source file!\n");

	if (source.linked_section.node_list.count)
		print_riot_bin_node(&source.linked_section, 0);

	print_riot_bin_node(&source.entries_section, 0);

	if (source.patches_section.node_map.count)
		print_riot_bin_node(&source.patches_section, 0);

	if ((err = riot_bin_try_write(&source, &dst_buf, &dst_len)) != RIOT_IO_ERROR_OK) {
		errlog("Failed to write out file: %s\n", riot_io_error_str(err));
		return 1;
	}

	errlog("Successfully wrote out source file!\n");

	free(src_buf);
	free(dst_buf);

	return 0;
}
