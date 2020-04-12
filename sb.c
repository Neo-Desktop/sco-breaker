#include <libgen.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "h.h"

static char	*progname;

void
usage(bool fail)
{
	fprintf(fail ? stderr : stdout,
			"usage: %s product_id major_ver minor_ver "
				"[license_data]\n"
			"       %s -r serial_number host_id\n"
			"       %s -r registration_lock\n",
			progname, progname, progname);
}

_Noreturn void
die(const char *msgfmt, ...)
{
	va_list	ap;

	fprintf(stderr, "%s: ", progname);

	va_start(ap, msgfmt);
	vfprintf(stderr, msgfmt, ap);
	va_end(ap);

	putchar('\n');
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	int	c;
	int	ret = EXIT_SUCCESS;
	bool	want_regcode = false;

	progname = basename((argv[0] != NULL) ? argv[0] : "sb");

	while ((c = getopt(argc, argv, "hr")) != -1) {
		switch (c) {
		case 'r':
			want_regcode = true;
			break;

		default:
			ret = EXIT_FAILURE;
		case 'h':
			usage(ret == EXIT_FAILURE);
			return ret;
		}
	}

	argc -= optind;
	argv += optind;

	if (want_regcode)
		ret = gen_regcode(argc, argv);
	else
		ret = gen_snak(argc, argv);

	return ret;
}

