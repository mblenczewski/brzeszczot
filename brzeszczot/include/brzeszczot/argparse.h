#ifndef BRZESZCZOT_ARGPARSE_H
#define BRZESZCZOT_ARGPARSE_H

#include "brzeszczot.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum brzeszczot_mode {
	WAD_DUMP,
	INIBIN_DUMP,
};

struct opts {
	enum brzeszczot_mode mode;
	char const *src, *dst;
};

extern b32
argparse(s32 argc, char **argv, struct opts *out);

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* BRZESZCZOT_ARGPARSE_H */
