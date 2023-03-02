#include <stdio.h>
#include <malloc.h>

/*
 * Matthias Gerstner
 * SUSE Linux GmbH
 * matthias.gerstner@suse.com
 *
 * A simple program that only prints the addresses of various types of
 * variables, constants and functions in address space.
 *
 * Compare this to /proc/<pid>/maps.
 */

const int const_global[] = { 1, 2, 3 };

int var_global[10] = { 0 };

#define HEAP_DATA_LEN 1024

int main() {
	// stack variables are implicitly of the C storage class 'auto' in
	// contrast to e.g. 'static'.
	int stack_var = 4711;
	char *heap_data = malloc(sizeof(char) * HEAP_DATA_LEN);

	printf("memory addresses\n");
	printf("----------------\n\n");
	printf("main() function = %p\n", main);
	printf("const_global    = %p\n", const_global);
	printf("var_global      = %p\n", var_global);
	printf("stack_var       = %p\n", &stack_var);
	printf("heap_data       = %p\n", heap_data);

	// just block on stdin for input so we can inspect the process while
	// it's running
	printf("\nwaiting for newline to end execution\n");
	fflush(stdout);
	fgets(heap_data, HEAP_DATA_LEN, stdin);

	free(heap_data);

	return 0;
}
