#ifndef LIBRIOT_H
#define LIBRIOT_H

#include "common.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef s32 riot_relptr_t;
typedef u32 riot_offptr_t;

struct riot_intrusive_list_node {
	riot_relptr_t prev, next;
};

extern inline void
riot_intrusive_list_node_link(struct riot_intrusive_list_node *self,
			      struct riot_intrusive_list_node *prev,
			      struct riot_intrusive_list_node *next);

extern inline void
riot_intrusive_list_node_snip(struct riot_intrusive_list_node *self);

typedef u32 fnv1a_u32;
typedef u64 xxh64_u64;

struct riot_fvec2 {
	f32 vs[2];
};

struct riot_fvec3 {
	f32 vs[3];
};

struct riot_fvec4 {
	f32 vs[4];
};

struct riot_fmat4x4 {
	f32 vs[16];
};

struct riot_rgba {
	u8 vs[4];
};

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* LIBRIOT_H */
