#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "md5.h"
#include "h.h"

static const char	*extSecret	= "\x5e\x4f\xbe\x45\x5e\x4c\x8d\x40\x9f\xeb\x26\x5e\x4f\xbe\x45\x5e\x4c\x3d\x30\x7c";

void
mdsnakext(const char *serno, const char *actkey, const char *snakext,
		char snakextmd[static 7])
{
	char *canon;

	canon = BSCanon(snakext);
	extmd(snakextmd, 4, extSecret, serno, actkey, canon);
	free(canon);
}

