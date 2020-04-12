#include <ctype.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "h.h"

static bool
overflow15(uint32_t a, uint32_t b)
{
	return (a + b > 0x7FFF);
}

static char *
mnsnc(const char *s)
{
	static char	buf[3];
	uint16_t	a;
	uint16_t	flag;
	int		c = s[8] % 16;

	for (a = 0; *s != '\0'; ++s) {
		if (overflow15(a, *s))
			flag = 1;
		else
			flag = 0;
		a = flag | (2 * (a + *s));
	}

	for (; c > 0; --c)
		a = ((a & 0x8000U) >> 15) | (uint16_t)(2 * a);

	buf[2] = 0;
	buf[1] = a % 26 + 'a';
	a /= 26;
	buf[0] = a % 26 + 'a';

	return buf;
}

static void
strbn(char *out, unsigned int in)
{
	static const char	*alphabet = "abcdefghijklmnopqrstuvwxyz";
	size_t			i;

	for (i = 3; i > 0; --i) {
		out[i - 1] = alphabet[in % 26];
		in /= 26;
	}
}

static void
decfrp(char *s)
{
	char		*p;
	unsigned char	a, b;

	a = b = 0;

	for (p = s + strlen(s) - 1; p >= s; --p) {
		a = (*p - 'a' + b) % 26;
		a = (a + 'a') & 0xFF;
		b += a;
		b %= 26;
		*p = a;
	}
}

/* Implementation of xorshift* without retaining seed state. */
static uint64_t
rnd(uint64_t seed)
{
	seed ^= seed >> 12;
	seed ^= seed << 25;
	seed ^= seed >> 27;
	return seed * 0x2545F4914F6CDD1D;
}

static unsigned int
mkver(unsigned int lictype, unsigned int major, unsigned int minor)
{
	return ((lictype << 12) | (major * 10 + minor));
}

static void
mksnak(bool has_snakext, uint16_t product_id, uint16_t major, uint16_t minor,
		char *serno, char *actkey)
{
	const char	*cksum;
	uint64_t 	serial;
	unsigned int	version = mkver((has_snakext ? 3 : 2), major, minor);
	char		merged[18];

	memset(serno, 0, 10);
	memset(actkey, 0, 9);
#ifdef DBG
	printf("has_snakext: %d, major: %u, minor: %u, version: %u\n",
			!!has_snakext, major, minor, version);
#endif

	/* bitmask to ensure at most six digits */
	serial = rnd((uintptr_t)serno * time(NULL)) & 0xEFFFF;
	snprintf(serno, 10, "SCO%06" PRIu64, serial);

	strbn(actkey, product_id);
	strbn(actkey + 3, version);
	snprintf(merged, sizeof(merged), "%s%s", serno, actkey);

	cksum = mnsnc(merged);
	actkey[6] = cksum[0];
	actkey[7] = cksum[1];

	decfrp(actkey);
}

static uint16_t
strtou16lim(const char *in, unsigned long limit)
{
	char		*end;
	unsigned long	i;

	if (limit > UINT16_MAX)
		die("internal: limit > UINT16_MAX");

	i = strtoul(in, &end, 10);
	if (*in == '\0' || *end != '\0')
		die("%s not a number", in);
	if (i > limit)
		die("%s out of range (max %"PRIu16")", in, limit);

	return i;
}

int
gen_snak(int argc, char *argv[])
{
	char		*snakext = NULL;
	uint16_t	product_id, major, minor;
	char		serno[10], actkey[9], snakextmd[7];

	if (argc < 3) {
		usage(true);
		return EXIT_FAILURE;
	}

	/* "zzz" is the maximum possible product ID encoded value.
	 * This decodes to a value of 17575.
	 */
	product_id = strtou16lim(argv[0], 17575);

	/* License type (whether snakext is to be read), version major and
	 * version minor share an integer, max encoded as "zzz".
	 * The license type is shifted up by 12, leaving 0xFFF (4095) for the
	 * version major and minor.
	 * Of that, the version major is all the upper digits, and the minor is
	 * the bottom digit (i.e. version/10 => major, version%10 => minor).
	 *
	 * Without doing too much checking, the maximum major version is 409 and
	 * the maximum minor version is 9.
	 */
	major = strtou16lim(argv[1], 409);
	minor = strtou16lim(argv[2], 9);

	if (argc >= 4)
		snakext = argv[3];

	mksnak(snakext != NULL, product_id, major, minor, serno, actkey);
	printf("Serial number:  %s\n"
			"Activation key: %s\n", serno, actkey);

	if (snakext != NULL) {
		mdsnakext(serno, actkey, snakext, snakextmd);
		printf("License data:   %s;m%s\n", snakext, snakextmd);
	}

	return EXIT_SUCCESS;
}

