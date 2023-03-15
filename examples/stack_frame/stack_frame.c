#include <stdio.h>
#include <stdint.h>

/*
 * Matthias Gerstner
 * SUSE Linux GmbH
 * matthias.gerstner@suse.com
 *
 * This is a simple program to explain the basics of function call and stack
 * frame handling.
 *
 * Inspect the generated assembler code, for example in gdb, step through the
 * program flow and inspect register values and what happens on the stack.
 *
 * Note: The register keyword asks the compiler to put the variable into a
 * register. But this is not mandatory for the compiler, just a suggestion.
 */

uint32_t somefunc(uint32_t counter) {
	uint32_t somevar = 0x4711;
	register int i = counter;
	// avoid compiler warning about an unused variable.
	(void)somevar;

	// just some loop for getting more interesting assembler output
	while (i != 0) {
		printf("%d.", i);
		i--;
	}

	return i;
}

int main() {
	uint32_t counter = 0x16;
	uint32_t ret;

	ret = somefunc(counter);
	(void)ret;

	return 0;
}
