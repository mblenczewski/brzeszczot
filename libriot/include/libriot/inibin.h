#ifndef LIBRIOT_INIBIN_H
#define LIBRIOT_INIBIN_H

#include "common.h"
#include "utils.h"

#include "libriot.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define RIOT_INIBIN_NODE_COMPLEX_TYPE_FLAG 0x80

enum riot_inibin_node_type {
	RIOT_INIBIN_NODE_NONE		= 0,

	RIOT_INIBIN_NODE_B8		= 1,
	RIOT_INIBIN_NODE_S8		= 2,
	RIOT_INIBIN_NODE_U8		= 3,
	RIOT_INIBIN_NODE_S16		= 4,
	RIOT_INIBIN_NODE_U16		= 5,
	RIOT_INIBIN_NODE_S32		= 6,
	RIOT_INIBIN_NODE_U32		= 7,
	RIOT_INIBIN_NODE_S64		= 8,
	RIOT_INIBIN_NODE_U64		= 9,
	RIOT_INIBIN_NODE_F32		= 10,
	RIOT_INIBIN_NODE_FVEC2		= 11,
	RIOT_INIBIN_NODE_FVEC3		= 12,
	RIOT_INIBIN_NODE_FVEC4		= 13,
	RIOT_INIBIN_NODE_FMAT4X4	= 14,
	RIOT_INIBIN_NODE_RGBA		= 15,
	RIOT_INIBIN_NODE_STR		= 16,
	RIOT_INIBIN_NODE_HASH		= 17,
	RIOT_INIBIN_NODE_FILE		= 18,

	RIOT_INIBIN_NODE_LIST		= 0 | RIOT_INIBIN_NODE_COMPLEX_TYPE_FLAG,
	RIOT_INIBIN_NODE_LIST2		= 1 | RIOT_INIBIN_NODE_COMPLEX_TYPE_FLAG,
	RIOT_INIBIN_NODE_PTR		= 2 | RIOT_INIBIN_NODE_COMPLEX_TYPE_FLAG,
	RIOT_INIBIN_NODE_EMBED		= 3 | RIOT_INIBIN_NODE_COMPLEX_TYPE_FLAG,
	RIOT_INIBIN_NODE_LINK		= 4 | RIOT_INIBIN_NODE_COMPLEX_TYPE_FLAG,
	RIOT_INIBIN_NODE_OPT		= 5 | RIOT_INIBIN_NODE_COMPLEX_TYPE_FLAG,
	RIOT_INIBIN_NODE_MAP		= 6 | RIOT_INIBIN_NODE_COMPLEX_TYPE_FLAG,
	RIOT_INIBIN_NODE_FLAG		= 7 | RIOT_INIBIN_NODE_COMPLEX_TYPE_FLAG,
};

struct riot_inibin_str {
	u16 count;
	riot_offptr_t data;
};

struct riot_inibin_field {
	fnv1a_u32 name_hash;
	riot_offptr_t value;
	struct riot_intrusive_list_node list;
};

struct riot_inibin_field_list {
	fnv1a_u32 name_hash;
	u16 count;
	riot_offptr_t root_field;
};

struct riot_inibin_opt {
	b8 exists;
	riot_relptr_t value;
};

struct riot_inibin_list {
	u32 count;
	riot_offptr_t root_node;
};

struct riot_inibin_pair {
	riot_offptr_t key, val;
	struct riot_intrusive_list_node list;
};

struct riot_inibin_map {
	u32 count;
	riot_offptr_t root_pair;
};

union riot_inibin_node_tag {
	b8 node_b8, node_flag;
	s8 node_s8;
	u8 node_u8;
	s16 node_s16;
	u16 node_u16;
	s32 node_s32;
	u32 node_u32;
	s64 node_s64;
	u64 node_u64;
	f32 node_f32;
	struct riot_fvec2 node_fvec2;
	struct riot_fvec3 node_fvec3;
	struct riot_fvec4 node_fvec4;
	struct riot_fmat4x4 node_fmat4x4;
	struct riot_rgba node_rgba;
	struct riot_inibin_str node_str;
	fnv1a_u32 node_hash, node_link;
	xxh64_u64 node_file;
	struct riot_inibin_field_list node_ptr, node_embed;
	struct riot_inibin_list node_list;
	struct riot_inibin_opt node_opt;
	struct riot_inibin_map node_map;
};

struct riot_inibin_node {
	enum riot_inibin_node_type type;
	union riot_inibin_node_tag tag;
	struct riot_intrusive_list_node list;
};

#define RIOT_INIBIN_CTX_STR_POOL_SZ 32 * KiB
#define RIOT_INIBIN_CTX_FIELD_POOL_SZ 8 * KiB
#define RIOT_INIBIN_CTX_PAIR_POOL_SZ 8 * KiB
#define RIOT_INIBIN_CTX_NODE_POOL_SZ 8 * KiB

struct riot_inibin_ctx {
	struct mem_pool str_pool, field_pool, pair_pool, node_pool;
};

extern b32
riot_inibin_ctx_init(struct riot_inibin_ctx *self);

extern void
riot_inibin_ctx_free(struct riot_inibin_ctx *self);

extern b32
riot_inibin_ctx_push_str(struct riot_inibin_ctx *self, u16 len, riot_offptr_t *out);

extern b32
riot_inibin_ctx_pushn_field(struct riot_inibin_ctx *self, u16 count, riot_offptr_t *out);

extern b32
riot_inibin_ctx_pushn_pair(struct riot_inibin_ctx *self, u32 count, riot_offptr_t *out);

extern b32
riot_inibin_ctx_pushn_node(struct riot_inibin_ctx *self, u32 count, riot_offptr_t *out);

extern b32
riot_inibin_read(struct riot_inibin_ctx *ctx, struct mem_stream stream);

extern b32
riot_inibin_write(struct riot_inibin_ctx *ctx, struct mem_stream stream);

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* LIBRIOT_INIBIN_H */
