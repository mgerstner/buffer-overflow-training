#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Matthias Gerstner
 * SUSE Linux GmbH
 * matthias.gerstner@suse.com
 *
 * Example of a vulnerable program.
 *
 * This program attempts to start a program on your behalf, but only one of a
 * defined subset that you may select by number via stdin.
 *
 * Imagine this would be a setuid-root program running with root privileges.
 */

const char* const ALLOWED_PROGS[] = {
	"/usr/bin/ls",
	"/usr/bin/who"
};

void runprog() {
	const char* const default_prog = ALLOWED_PROGS[0];
	const char *prog_to_run = default_prog;
	char selection[32] = {0};
	char *parsing_end = NULL;
	unsigned long index = 0;

	// this is just a little help for exploiting this program
	printf("[selection ptr is at %p]\n", selection);

	printf("Allowed programs:\n\n");
	for (int i = 0; i < sizeof(ALLOWED_PROGS) / sizeof(ALLOWED_PROGS[0]); i++) {
		printf("%d: %s\n", i, ALLOWED_PROGS[i]);
	}
	printf("default program is: %s\n", default_prog);

	printf("\nenter program index to run > ");
	fflush(stdout);

	scanf("%s", selection);
	index = strtoul(selection, &parsing_end, 10);

	if (parsing_end == selection) {
		fprintf(stderr, "\n>>> non-numeric input, using default prog\n");

	} else if (index < sizeof(ALLOWED_PROGS) / sizeof(char*)) {
		prog_to_run = ALLOWED_PROGS[index];

	} else {
		fprintf(stderr, "\n>>> invalid program index, using default prog\n");
	}

	printf("\n============\nRunning program '%s'\n============\n", prog_to_run);

	execl(prog_to_run, prog_to_run, NULL);

	fprintf(stderr, ">>> Failed to run program!\n");
}

int main() {
	runprog();

	return 0;
}

