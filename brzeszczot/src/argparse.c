#include "brzeszczot/argparse.h"

void
usage(s32 argc, char **argv) {
	(void) argc;

	fprintf(stderr, "Usage: %s <src-file> <dst-file> <wad|inibin>\n", argv[0]);
}

b32
argparse(s32 argc, char **argv, struct opts *out) {
	assert(out);

	(void) argc;
	(void) argv;

	if (argc < 4) {
		usage(argc, argv);
		return false;
	}

	out->src = argv[1];
	out->dst = argv[2];

	if (strcmp(argv[3], "wad") == 0) {
		out->mode = WAD_DUMP;
	} else if (strcmp(argv[3], "inibin") == 0) {
		out->mode = INIBIN_DUMP;
	} else {
		usage(argc, argv);
		return false;
	}

	return true;
}
