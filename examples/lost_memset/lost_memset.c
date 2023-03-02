#include <stdio.h>
#include <string.h>
#include <malloc.h>

/*
 * Matthias Gerstner
 * SUSE Linux GmbH
 * matthias.gerstner@suse.com
 */

/*
 * This program shows how a call to memset() is optimized out by typical
 * compilers
 */

const size_t MAX_LEN = 128;

int main(int argc, char **argv) {
	char secret[MAX_LEN];

	if (argc != 2) {
		fprintf(stderr, "%s: <string>\n", argv[0]);
		return 1;
	}

	/*
	 * pretend here something sensible would be done with the string
	 */

	if (snprintf(secret, MAX_LEN, "%s", argv[1]) >= MAX_LEN) {
		fprintf(stderr, "secret string too long, maximum length = %zd\n", MAX_LEN);
		return 1;
	}

	printf("Your secret string '%s' will be cleaned now\n", secret);

	memset(secret, 0, MAX_LEN);

	return 0;
}
