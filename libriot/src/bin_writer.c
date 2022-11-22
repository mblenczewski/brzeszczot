#include "libriot/io.h"

enum riot_io_error
riot_bin_try_write(struct riot_bin const *val, u8 **buf, size_t *len) {
	assert(val);
	assert(buf);
	assert(len);

	return RIOT_IO_ERROR_OK;
}

