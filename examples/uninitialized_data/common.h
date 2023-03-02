#include <stdio.h>
#include <stdlib.h>

/*
 * Matthias Gerstner
 * SUSE Linux GmbH
 * matthias.gerstner@suse.com
 */

unsigned long int parse_integer(const char *arg, size_t base) {
	unsigned long int ret = 0;
	char *endptr = NULL;

	ret = strtoul(arg, &endptr, base);

	if (endptr == arg) {
		fprintf(stderr, "Failed to parse integer from '%s'.\n", arg);
		exit(1);
	}

	return ret;
}
