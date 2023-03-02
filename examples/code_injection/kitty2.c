#include <stdio.h>
#include <string.h>

/*
 * Matthias Gerstner
 * SUSE Linux GmbH
 * matthias.gerstner@suse.com
 */

/*
 * One more badly implemented cat like program.
 *
 * It accepts input either from stdin or a file passed as parameter argv[1].
 */

void cat(FILE *input) {
	char text[256];
	int res;

	printf("[address of text buffer: %p]\n", text);

	while (1) {
		// read a complete line into the text buffer
		res = fscanf(input, " %[^\n]", text);

		if (res == EOF) {
			return;
		} else if (res != 1) {
			fprintf(stderr, "bad input\n");
			return;
		}

		printf("%s\n", text);
	}
}

int main(const int argc, const char **argv) {
	// open file specified on command line or stdin by default
	FILE *input = (argc == 2) ? fopen(argv[1], "r") : stdin;

	if (input == NULL) {
		fprintf(stderr, "bad input file\n");
		return 1;
	}

	cat(input);

	return 0;
}
