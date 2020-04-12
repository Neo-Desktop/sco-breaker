#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "h.h"
#include "md5.h"

static const char	*regSecret = "\x75\xf8\xe8\x5e\x83\xc4\x5e\x4c\xff\x75\x5e\x48\xe8\x65\x5e\x46\x59\x8b\x45";

static char *
toAsciiBase16(uint32_t in, unsigned char ckbase)
{
	static const char	*charmap	= "\005k\001b\002w\003t\004a\005c\001o\002r\003h\004z\005g\001s\002e\003j\004q\005x\001";
	static char		buf[11];
	unsigned int		a, i;
	unsigned char		ck;

	ck = ckbase;
	for (i = 8; i > 0; --i) {
		a = 15 - (in & 0xF);
		buf[i - 1] = charmap[2 * a + 1];
		ck += a * (i);
		in >>= 4;
	}

	/* checksum */
	for (i = 9; i >= 8; --i) {
		buf[i] = charmap[2 * (ck & 0xF) + 1];
		ck >>= 4;
	}

	buf[10] = '\0';
	return buf;
}

static uint32_t
l32be(uint8_t i[4])
{
	return (uint32_t)i[3]
		| ((uint32_t)i[2] <<  8)
		| ((uint32_t)i[1] << 16)
		| ((uint32_t)i[0] << 24);
}

static char *
generateRegistrationID(const char *serno, const char *szHostid)
{
	static char		buf[11];
	uint32_t 		regkey;
	MD5_CTX			ctx;
	uint8_t			digest[MD5_DIGEST_LENGTH];

	MD5Init(&ctx);
	MD5Update(&ctx, (const uint8_t *)regSecret, strlen(regSecret));
	MD5Update(&ctx, (const uint8_t *)szHostid, strlen(szHostid));
	MD5Update(&ctx, (const uint8_t *)serno, strlen(serno));
	MD5Final(digest, &ctx);

	regkey = l32be(digest);
	snprintf(buf, sizeof(buf), "%s", toAsciiBase16(regkey, 3));
	return buf;
}

static bool
valid_reglock(char *reglock)
{
	char	*canon, *theirs, *p;
	char	mine[7];

	if ((p = strstr(reglock, ";m")) == NULL)
		return false;
	if (*(p + 2) == '\0')
		return false;

	*p = '\0';
	theirs = p + 2;

	canon = BSCanon(reglock);
	extmd(mine, 2, regSecret, canon);
	free(canon);

	return (strcmp(mine, theirs) == 0);
}

static void
parse_reglock(char *reglock, char **serno, char **hostid)
{
	char	*last, *p;

	if (!valid_reglock(reglock))
		die("registration lock %s invalid; check for typos", reglock);

	/* Assumption: Nobody generates bogus reglocks, so they're well-behaved
	 * after the MD5 checks out.
	 */
	for (p = strtok_r(reglock, ";", &last);
			p != NULL;
			p = strtok_r(NULL, ";", &last)) {
		switch (*p) {
		case 'o':
			*serno = p + 1;
			break;
		case 'u':
			*hostid = p + 1;
			break;
		default:
			break;
		}
	}
}

int
gen_regcode(int argc, char *argv[])
{
	char	*serno, *hostid, *regcode;
	size_t	serno_len, hostid_len;

	if (argc < 1) {
		usage(true);
		return EXIT_FAILURE;
	}

	if (argc == 2) {
		serno = argv[0];
		hostid = argv[1];
		serno_len = strlen(serno);
		hostid_len = strlen(hostid);
		if (serno_len == 10 && hostid_len == 9) {
			/* Arguments are probably swapped. */
			serno = argv[1];
			hostid = argv[0];
		} else if (serno_len != 9 || hostid_len != 10) {
			die("invalid length for serno or hostid");
		}
	} else {
		parse_reglock(argv[0], &serno, &hostid);
	}

	regcode = generateRegistrationID(serno, hostid);
	printf("Registration Key: %s\n", regcode);

	return EXIT_SUCCESS;
}


