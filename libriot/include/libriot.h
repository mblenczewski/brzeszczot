#ifndef LIBRIOT_H
#define LIBRIOT_H

#include "common.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef s32 riot_bin_relptr_t;
typedef u32 riot_bin_offptr_t;

struct riot_bin_intrusive_list_node {
	riot_bin_relptr_t prev, next;
};

extern inline void
riot_bin_intrusive_list_node_link(struct riot_bin_intrusive_list_node *self,
				  struct riot_bin_intrusive_list_node *prev,
				  struct riot_bin_intrusive_list_node *next);

extern inline void
riot_bin_intrusive_list_node_snip(struct riot_bin_intrusive_list_node *self);

#define RIOT_BIN_NODE_COMPLEX_TYPE_FLAG 0x80

enum riot_bin_node_type {
	RIOT_BIN_NODE_NONE	= 0,

	RIOT_BIN_NODE_B8	= 1,
	RIOT_BIN_NODE_S8	= 2,
	RIOT_BIN_NODE_U8	= 3,
	RIOT_BIN_NODE_S16	= 4,
	RIOT_BIN_NODE_U16	= 5,
	RIOT_BIN_NODE_S32	= 6,
	RIOT_BIN_NODE_U32	= 7,
	RIOT_BIN_NODE_S64	= 8,
	RIOT_BIN_NODE_U64	= 9,
	RIOT_BIN_NODE_F32	= 10,
	RIOT_BIN_NODE_FVEC2	= 11,
	RIOT_BIN_NODE_FVEC3	= 12,
	RIOT_BIN_NODE_FVEC4	= 13,
	RIOT_BIN_NODE_FMAT4X4	= 14,
	RIOT_BIN_NODE_RGBA	= 15,
	RIOT_BIN_NODE_STR	= 16,
	RIOT_BIN_NODE_HASH	= 17,
	RIOT_BIN_NODE_FILE	= 18,

	RIOT_BIN_NODE_LIST	= 0 | RIOT_BIN_NODE_COMPLEX_TYPE_FLAG,
	RIOT_BIN_NODE_LIST2	= 1 | RIOT_BIN_NODE_COMPLEX_TYPE_FLAG,
	RIOT_BIN_NODE_PTR	= 2 | RIOT_BIN_NODE_COMPLEX_TYPE_FLAG,
	RIOT_BIN_NODE_EMBED	= 3 | RIOT_BIN_NODE_COMPLEX_TYPE_FLAG,
	RIOT_BIN_NODE_LINK	= 4 | RIOT_BIN_NODE_COMPLEX_TYPE_FLAG,
	RIOT_BIN_NODE_OPT	= 5 | RIOT_BIN_NODE_COMPLEX_TYPE_FLAG,
	RIOT_BIN_NODE_MAP	= 6 | RIOT_BIN_NODE_COMPLEX_TYPE_FLAG,
	RIOT_BIN_NODE_FLAG	= 7 | RIOT_BIN_NODE_COMPLEX_TYPE_FLAG,
};

typedef u32 fnv1a_u32;
typedef u64 xxh64_u64;

struct riot_fvec2 { f32 vs[2]; };
struct riot_fvec3 { f32 vs[3]; };
struct riot_fvec4 { f32 vs[4]; };
struct riot_fmat4x4 { f32 vs[16]; };
struct riot_rgba { u8 vs[4]; };

struct riot_bin_str {
	u16 count;
	riot_bin_offptr_t data;
};

struct riot_bin_field {
	fnv1a_u32 name_hash;
	riot_bin_offptr_t value;
	struct riot_bin_intrusive_list_node list;
};

struct riot_bin_field_list {
	fnv1a_u32 name_hash;
	u16 count;
	riot_bin_offptr_t root;
};

struct riot_bin_opt {
	b8 exists;
	riot_bin_relptr_t value;
};

struct riot_bin_list {
	u32 count;
	riot_bin_offptr_t root;
};

struct riot_bin_pair {
	riot_bin_offptr_t key, val;
	struct riot_bin_intrusive_list_node list;
};

struct riot_bin_map {
	u32 count;
	riot_bin_offptr_t root;
};

union riot_bin_node_tag {
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
	struct riot_bin_str node_str;
	fnv1a_u32 node_hash, node_link;
	xxh64_u64 node_file;
	struct riot_bin_field_list node_ptr, node_embed;
	struct riot_bin_list node_list;
	struct riot_bin_opt node_opt;
	struct riot_bin_map node_map;
};

struct riot_bin_node {
	enum riot_bin_node_type type;
	union riot_bin_node_tag tag;
	struct riot_bin_intrusive_list_node list;
};

#define RIOT_BIN_CTX_STR_POOL_SZ 32 * KiB
#define RIOT_BIN_CTX_FIELD_POOL_SZ 8 * KiB
#define RIOT_BIN_CTX_PAIR_POOL_SZ 8 * KiB
#define RIOT_BIN_CTX_NODE_POOL_SZ 8 * KiB

struct riot_bin_ctx {
	struct mem_pool str_pool, field_pool, pair_pool, node_pool;
};

extern b32
riot_bin_ctx_init(struct riot_bin_ctx *self);

extern void
riot_bin_ctx_free(struct riot_bin_ctx *self);

extern b32
riot_bin_ctx_push_str(struct riot_bin_ctx *self, u16 len, riot_bin_offptr_t *out);

extern b32
riot_bin_ctx_pushn_field(struct riot_bin_ctx *self, u16 count, riot_bin_offptr_t *out);

extern b32
riot_bin_ctx_pushn_pair(struct riot_bin_ctx *self, u32 count, riot_bin_offptr_t *out);

extern b32
riot_bin_ctx_pushn_node(struct riot_bin_ctx *self, u32 count, riot_bin_offptr_t *out);

extern b32
riot_bin_read(struct riot_bin_ctx *ctx, struct mem_stream stream);

extern b32
riot_bin_write(struct riot_bin_ctx *ctx, struct mem_stream stream);

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* LIBRIOT_H */
