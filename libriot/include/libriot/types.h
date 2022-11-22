#ifndef LIBRIOT_TYPES_H
#define LIBRIOT_TYPES_H

#include "libriot.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define RIOT_BIN_NODE_TYPE_COMPLEX_FLAG 0x80

enum __attribute__((packed)) riot_bin_node_type {
	RIOT_BIN_NODE_TYPE_NONE		= 0,

	/* primitive types */
	RIOT_BIN_NODE_TYPE_B8		= 1,
	RIOT_BIN_NODE_TYPE_I8		= 2,
	RIOT_BIN_NODE_TYPE_U8		= 3,
	RIOT_BIN_NODE_TYPE_I16		= 4,
	RIOT_BIN_NODE_TYPE_U16		= 5,
	RIOT_BIN_NODE_TYPE_I32		= 6,
	RIOT_BIN_NODE_TYPE_U32		= 7,
	RIOT_BIN_NODE_TYPE_I64		= 8,
	RIOT_BIN_NODE_TYPE_U64		= 9,
	RIOT_BIN_NODE_TYPE_F32		= 10,
	RIOT_BIN_NODE_TYPE_VEC2		= 11,
	RIOT_BIN_NODE_TYPE_VEC3		= 12,
	RIOT_BIN_NODE_TYPE_VEC4		= 13,
	RIOT_BIN_NODE_TYPE_MAT4		= 14,
	RIOT_BIN_NODE_TYPE_RGBA		= 15,
	RIOT_BIN_NODE_TYPE_STR		= 16,
	RIOT_BIN_NODE_TYPE_HASH		= 17,
	RIOT_BIN_NODE_TYPE_FILE		= 18,

	/* complex types */
	RIOT_BIN_NODE_TYPE_LIST		= 0 | RIOT_BIN_NODE_TYPE_COMPLEX_FLAG,
	RIOT_BIN_NODE_TYPE_LIST2	= 1 | RIOT_BIN_NODE_TYPE_COMPLEX_FLAG,
	RIOT_BIN_NODE_TYPE_PTR		= 2 | RIOT_BIN_NODE_TYPE_COMPLEX_FLAG,
	RIOT_BIN_NODE_TYPE_EMBED	= 3 | RIOT_BIN_NODE_TYPE_COMPLEX_FLAG,
	RIOT_BIN_NODE_TYPE_LINK		= 4 | RIOT_BIN_NODE_TYPE_COMPLEX_FLAG,
	RIOT_BIN_NODE_TYPE_OPTION	= 5 | RIOT_BIN_NODE_TYPE_COMPLEX_FLAG,
	RIOT_BIN_NODE_TYPE_MAP		= 6 | RIOT_BIN_NODE_TYPE_COMPLEX_FLAG,
	RIOT_BIN_NODE_TYPE_FLAG		= 7 | RIOT_BIN_NODE_TYPE_COMPLEX_FLAG,
};

struct riot_vec2 { f32 vs[2]; };
struct riot_vec3 { f32 vs[3]; };
struct riot_vec4 { f32 vs[4]; };
struct riot_mat4x4 { f32 vs[16]; };
struct riot_rgba { u8 vs[4]; };
struct riot_str { u16 len; char *ptr; };

struct riot_bin_node;
struct riot_bin_pair;
struct riot_bin_field;

struct riot_bin_node_list {
	enum riot_bin_node_type ty;
	u32 count;
	struct riot_bin_node *vs;
};

struct riot_bin_node_option {
	enum riot_bin_node_type ty;
	struct riot_bin_node *vp;
};

struct riot_bin_node_map {
	enum riot_bin_node_type key_ty, val_ty;
	u32 count;
	struct riot_bin_pair *vs;
};

struct riot_bin_field_list {
	hashes_fnv1a_val_t hash;
	u16 count;
	struct riot_bin_field *vs;
};

struct riot_bin_node {
	enum riot_bin_node_type type;
	union {
		/* generic member data pointer */
		u8 raw_data;

		/* primitive members */
		u8 node_bool, node_flag;
		s8 node_i8;
		u8 node_u8;
		s16 node_i16;
		u16 node_u16;
		s32 node_i32;
		u32 node_u32;
		s64 node_i64;
		u64 node_u64;
		f32 node_f32;
		struct riot_vec2 node_vec2;
		struct riot_vec3 node_vec3;
		struct riot_vec4 node_vec4;
		struct riot_mat4x4 node_mat4;
		struct riot_rgba node_rgba;
		struct riot_str node_str;
		hashes_fnv1a_val_t node_hash, node_link;
		hashes_xxh64_val_t node_file;

		/* complex members */
		struct riot_bin_node_list node_list;
		struct riot_bin_node_option node_option;
		struct riot_bin_node_map node_map;
		struct riot_bin_field_list node_ptr, node_embed;
	};
};

struct riot_bin_pair {
	struct riot_bin_node key, val;
};

struct riot_bin_field {
	hashes_fnv1a_val_t hash;
	struct riot_bin_node val;
};

struct riot_bin_mempool {
	struct {
		char *ptr, *head;
		size_t len;
	} strings;

	struct {
		struct riot_bin_node *ptr, *head;
		size_t len;
	} nodes;

	struct {
		struct riot_bin_pair *ptr, *head;
		size_t len;
	} pairs;

	struct {
		struct riot_bin_field *ptr, *head;
		size_t len;
	} fields;
};

struct riot_bin {
	struct riot_bin_mempool mempool;
	struct riot_bin_node type_section, version_section, linked_section,
			     entries_section, patches_section;
};

static inline void
riot_bin_free(struct riot_bin *self) {
	assert(self);

	free(self->mempool.strings.ptr);
	free(self->mempool.nodes.ptr);
	free(self->mempool.pairs.ptr);
	free(self->mempool.fields.ptr);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBRIOT_TYPES_H */
