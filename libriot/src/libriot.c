#include "libriot.h"

b32
riot_bin_ctx_init(struct riot_bin_ctx *self) {
	assert(self);

	if (!MEM_POOL_INIT(&self->str_pool, char, RIOT_BIN_CTX_STR_POOL_SZ))
		goto str_pool_alloc_failure;

	if (!MEM_POOL_INIT(&self->field_pool, struct riot_bin_field, RIOT_BIN_CTX_FIELD_POOL_SZ))
		goto field_pool_alloc_failure;

	if (!MEM_POOL_INIT(&self->pair_pool, struct riot_bin_pair, RIOT_BIN_CTX_PAIR_POOL_SZ))
		goto pair_pool_alloc_failure;

	if (!MEM_POOL_INIT(&self->node_pool, struct riot_bin_node, RIOT_BIN_CTX_NODE_POOL_SZ))
		goto node_pool_alloc_failure;

	return true;

node_pool_alloc_failure:
	mem_pool_free(&self->pair_pool);
pair_pool_alloc_failure:
	mem_pool_free(&self->field_pool);
field_pool_alloc_failure:
	mem_pool_free(&self->str_pool);
str_pool_alloc_failure:
	return false;
}

void
riot_bin_ctx_free(struct riot_bin_ctx *self) {
	assert(self);

	mem_pool_free(&self->str_pool);
	mem_pool_free(&self->field_pool);
	mem_pool_free(&self->pair_pool);
	mem_pool_free(&self->node_pool);
}

b32
riot_bin_ctx_push_str(struct riot_bin_ctx *self, u16 len, riot_bin_offptr_t *out) {
	assert(self);

	void *absptr = MEM_POOL_ALLOC(&self->str_pool, char, len);
	if (!absptr) return false;

	*out = (char *)absptr - (char *)self->str_pool.ptr;

	return true;
}

b32
riot_bin_ctx_pushn_field(struct riot_bin_ctx *self, u16 count, riot_bin_offptr_t *out) {
	assert(self);

	void *absptr = MEM_POOL_ALLOC(&self->field_pool, struct riot_bin_field, count);
	if (!absptr) return false;

	*out = (struct riot_bin_field *)absptr - (struct riot_bin_field *)self->field_pool.ptr;

	return true;
}

b32
riot_bin_ctx_pushn_pair(struct riot_bin_ctx *self, u32 count, riot_bin_offptr_t *out) {
	assert(self);

	void *absptr = MEM_POOL_ALLOC(&self->pair_pool, struct riot_bin_pair, count);
	if (!absptr) return false;

	*out = (struct riot_bin_pair *)absptr - (struct riot_bin_pair *)self->pair_pool.ptr;

	return true;
}

b32
riot_bin_ctx_pushn_node(struct riot_bin_ctx *self, u32 count, riot_bin_offptr_t *out) {
	assert(self);

	void *absptr = MEM_POOL_ALLOC(&self->node_pool, struct riot_bin_node, count);
	if (!absptr) return false;

	*out = (struct riot_bin_node *)absptr - (struct riot_bin_node *)self->node_pool.ptr;

	return true;
}
