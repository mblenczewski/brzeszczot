#include "brzeszczot.h"
#include "brzeszczot/argparse.h"

static inline u64
read_file(char const *fp, u8 **out) {
	FILE *f = fopen(fp, "rb");
	if (!f) return 0;

	u64 len;
	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fseek(f, 0, SEEK_SET);

	u8 *buf = malloc(len);
	if (!buf) { fclose(f); return 0; }

	int fd = fileno(f);

	u64 total_read = 0;
	do {
		u64 curr = read(fd, buf + total_read, len - total_read);
		if (!curr) { free(buf); fclose(f); return 0; }

		total_read += curr;
	} while (total_read < len);

	fclose(f);

	*out = buf;

	return total_read;
}

static inline u64
write_file(char const *fp, u64 len, u8 *buf) {
	FILE *f = fopen(fp, "wb");
	if (!f) return 0;

	int fd = fileno(f);

	u64 total_written = 0;
	do {
		u64 curr = write(fd, buf + total_written, len - total_written);
		if (!curr) { fclose(f); return total_written; }

		total_written += curr;
	} while (total_written < len);

	fclose(f);

	return total_written;
}

static s32
wad_dump(struct opts *opts) {
	assert(opts);

	u8 *filebuf;
	u64 filelen = read_file(opts->src, &filebuf);
	if (!filelen) {
		errlog("Failed to read source file: %s", opts->src);
		return 1;
	}

	struct mem_stream in = {
		.ptr = filebuf,
		.len = filelen,
		.cur = 0,
	};

	struct riot_wad_ctx ctx;
	if (!riot_wad_ctx_init(&ctx)) {
		errlog("Failed to initialise WAD context");
		free(filebuf);
		return 1;
	}

	if (!riot_wad_read(&ctx, in)) {
		errlog("Failed to read WAD file");
		riot_wad_ctx_free(&ctx);
		free(filebuf);
		return 1;
	}

	riot_wad_print(&ctx, stdout);

	struct mem_stream out = {
		.ptr = filebuf,
		.len = filelen,
		.cur = 0,
	};

	void *wad_data_buf = filebuf + ctx.wad.data_start;
	u64 wad_data_len = filelen - ctx.wad.data_start;
	if (!riot_wad_write(&ctx, wad_data_buf, wad_data_len, out)) {
		errlog("Failed to write WAD file");
		riot_wad_ctx_free(&ctx);
		free(filebuf);
		return 1;
	}

	riot_wad_ctx_free(&ctx);

	u64 written = write_file(opts->dst, filelen, filebuf);
	if (!written || written < filelen) {
		errlog("Failed to write destination file: %s", opts->dst);
		free(filebuf);
		return 1;
	}

	free(filebuf);

	return 0;
}

static s32
inibin_dump(struct opts *opts) {
	assert(opts);

	u8 *filebuf;
	u64 filelen = read_file(opts->src, &filebuf);
	if (!filelen) {
		errlog("Failed to read source file: %s", opts->src);
		return 1;
	}

	struct mem_stream in = {
		.ptr = filebuf,
		.len = filelen,
		.cur = 0,
	};

	struct riot_inibin_ctx ctx;
	if (!riot_inibin_ctx_init(&ctx)) {
		errlog("Failed to initialise INIBIN context");
		free(filebuf);
		return 1;
	}

	if (!riot_inibin_read(&ctx, in)) {
		errlog("Failed to read INIBIN file");
		riot_inibin_ctx_free(&ctx);
		free(filebuf);
		return 1;
	}

	riot_inibin_print(&ctx, stdout);

	struct mem_stream out = {
		.ptr = filebuf,
		.len = filelen,
		.cur = 0,
	};

	if (!riot_inibin_write(&ctx, out)) {
		errlog("Failed to write INIBIN file");
		riot_inibin_ctx_free(&ctx);
		free(filebuf);
		return 1;
	}

	riot_inibin_ctx_free(&ctx);

	u64 written = write_file(opts->dst, filelen, filebuf);
	if (!written || written < filelen) {
		errlog("Failed to write destination file: %s", opts->dst);
		free(filebuf);
		return 1;
	}

	free(filebuf);

	return 0;
}

s32
main(s32 argc, char **argv) {
	dbglog("Version: " BRZESZCZOT_VERSION);

	struct opts opts;
	if (!argparse(argc, argv, &opts)) return 1;

	switch (opts.mode) {
	case WAD_DUMP:
		return wad_dump(&opts);

	case INIBIN_DUMP:
		return inibin_dump(&opts);

	default:
		errlog("Unknown mode: %d", opts.mode);
		return 1;
	}
}
