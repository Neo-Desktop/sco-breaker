#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "md5.h"
#include "h.h"

const char	*fromHextet	= "0123456789abcdefghjkmnpqrstuwxyz";

static int
asciiCode(int c)
{
	static const int fromAlpha[] = {
		0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11, 1, 0x12, 0x13, 1,
		0x14, 0x15, 0, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1b,
		0x1c, 0x1d, 0x1e, 0x1f
	};

	if (c >= '0' && c <= '9')
		return c - '0';
	if (isupper(c))
		c = tolower(c);
	if (islower(c))
		return fromAlpha[c - 'a'];
	return -1;
}

char *
BSCanon(const char *s)
{
	char *ret;
	int c;

	ret = malloc(strlen(s) + 1);
	if (ret == NULL)
		die("malloc");
	strcpy(ret, s);

	for (char *p = ret; *p; ++p) {
		c = asciiCode(*p);
		if (c != -1)
			*p = fromHextet[c];
	}

	return ret;
}

void
extmd(char md[static 7], unsigned int nstr, ...)
{
	char *str;
	MD5_CTX ctx;
	char digest[MD5_DIGEST_LENGTH];
	va_list ap;

	MD5Init(&ctx);
	va_start(ap, nstr);

	for (unsigned int i = 0; i < nstr; ++i) {
		str = va_arg(ap, char *);
		MD5Update(&ctx, (const uint8_t *)str, strlen(str));
	}

	MD5Final(digest, &ctx);
	for (size_t i = 0; i < 6; ++i)
		md[i] = fromHextet[digest[i] & 0x1F];
	md[6] = '\0';
}

