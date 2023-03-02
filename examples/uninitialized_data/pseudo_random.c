#include <stdlib.h>
#include <stdio.h>

#include "common.h"

/*
 * Matthias Gerstner
 * SUSE Linux GmbH
 * matthias.gerstner@suse.com
 */

/*
 * This program simply prints the result of COUNT random() calls when seeding
 * the pseudo random generator with a given SEED.
 */

const size_t MAX_COUNT = 64;

int main(int argc, char **argv) {
	unsigned int seed = 0;
	size_t count = 0;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <seed> <count>\n", argv[0]);
		fprintf(stderr, "\nOutputs a series of <count> pseudo-random numbers based on <seed>\n"); 
		return 1;
	}

	seed = parse_integer(argv[1], 0);
	count = parse_integer(argv[2], 10);

	if (count > MAX_COUNT) {
		fprintf(stderr, "Requested count %zu exceeds maximum count %zu\n", count, MAX_COUNT);
		return 1;
	}

	srand(seed);

	while (count--) {
		printf("%u\n", rand());
	}

	return 0;
}
