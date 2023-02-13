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

inline void
riot_intrusive_list_node_link(struct riot_intrusive_list_node *self,
			      struct riot_intrusive_list_node *prev,
			      struct riot_intrusive_list_node *next) {
	assert(self);
	assert(prev);
	assert(next);

	prev->next = RELPTR_ABS2REL(riot_relptr_t, prev, self);
	self->prev = RELPTR_ABS2REL(riot_relptr_t, self, prev);
	self->next = RELPTR_ABS2REL(riot_relptr_t, self, next);
	next->prev = RELPTR_ABS2REL(riot_relptr_t, next, self);
}

inline void
riot_intrusive_list_node_snip(struct riot_intrusive_list_node *self) {
	assert(self);

	struct riot_intrusive_list_node *prev, *next;
	prev = RELPTR_REL2ABS(struct riot_intrusive_list_node *, riot_relptr_t, self, self->prev);
	next = RELPTR_REL2ABS(struct riot_intrusive_list_node *, riot_relptr_t, self, self->next);

	prev->next = RELPTR_ABS2REL(riot_relptr_t, prev, next);
	next->prev = RELPTR_ABS2REL(riot_relptr_t, next, prev);
	self->prev = self->next = RELPTR_NULL;
}

struct riot_intrusive_list {
	struct riot_intrusive_list_node root;
};

inline void
riot_intrusive_list_init(struct riot_intrusive_list *self) {
	assert(self);

	self->root.prev = self->root.next = RELPTR_ABS2REL(riot_relptr_t, &self->root, &self->root);
}

inline void
riot_intrusive_list_push(struct riot_intrusive_list *self,
			 struct riot_intrusive_list_node *elem) {
	assert(self);
	assert(elem);

	struct riot_intrusive_list_node *prev, *next;
	prev = RELPTR_REL2ABS(struct riot_intrusive_list_node *, riot_relptr_t, &self->root, self->root.prev);
	next = RELPTR_REL2ABS(struct riot_intrusive_list_node *, riot_relptr_t, &self->root, self->root.next);

	riot_intrusive_list_node_link(elem, prev, next);
}

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
