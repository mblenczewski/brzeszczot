#include "libriot/wad.h"

b32
riot_wad_ctx_init(struct riot_wad_ctx *self) {
	assert(self);

	if (!MEM_POOL_INIT(&self->chunk_pool, struct riot_wad_chunk, RIOT_WAD_CTX_CHUNK_POOL_SZ))
		goto chunk_pool_alloc_failure;

	return true;

chunk_pool_alloc_failure:
	return false;
}

void
riot_wad_ctx_free(struct riot_wad_ctx *self) {
	assert(self);

	mem_pool_free(&self->chunk_pool);
}

b32
riot_wad_ctx_pushn_chunk(struct riot_wad_ctx *self, u32 count, riot_offptr_t *out) {
	assert(self);
	assert(out);

	void *absptr = MEM_POOL_ALLOC(&self->chunk_pool, struct riot_wad_chunk, count);
	if (!absptr) return false;

	*out = (struct riot_wad_chunk *)absptr - (struct riot_wad_chunk *)self->chunk_pool.ptr;

	return true;
}
