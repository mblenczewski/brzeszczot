#include "libriot.h"

void
riot_bin_intrusive_list_node_link(struct riot_bin_intrusive_list_node *self,
				  struct riot_bin_intrusive_list_node *prev,
				  struct riot_bin_intrusive_list_node *next) {
	assert(self);
	assert(prev);
	assert(next);

	prev->next = RELPTR_ABS2REL(riot_bin_relptr_t, prev, self);
	self->prev = RELPTR_ABS2REL(riot_bin_relptr_t, self, prev);
	self->next = RELPTR_ABS2REL(riot_bin_relptr_t, self, next);
	next->prev = RELPTR_ABS2REL(riot_bin_relptr_t, next, self);
}

void
riot_bin_intrusive_list_node_snip(struct riot_bin_intrusive_list_node *self) {
	assert(self);

	struct riot_bin_intrusive_list_node *prev, *next;
	prev = RELPTR_REL2ABS(struct riot_bin_intrusive_list_node *, riot_bin_relptr_t, self, self->prev);
	next = RELPTR_REL2ABS(struct riot_bin_intrusive_list_node *, riot_bin_relptr_t, self, self->next);

	prev->next = RELPTR_ABS2REL(riot_bin_relptr_t, prev, next);
	next->prev = RELPTR_ABS2REL(riot_bin_relptr_t, next, prev);
	self->prev = self->next = RELPTR_NULL;
}
