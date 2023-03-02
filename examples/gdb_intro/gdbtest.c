#include <stdio.h>
#include <unistd.h>

/*
 * Matthias Gerstner
 * SUSE Linux GmbH
 * matthias.gerstner@suse.com
 */

void handle_parameter(const char *param) {
	unsigned int to_sleep = 600;

	printf("parameter: %s\n", param);
	printf("sleeping for %u seconds\n", to_sleep);

	sleep(to_sleep);
}

void usage(const char *program) {
	printf("Usage: %s <PARAMETER>\n", program);
}

int main(int argc, const char **argv) {

	if (argc == 2) {
		handle_parameter(argv[1]);
		return 0;
	} else {
		usage(argv[0]);
		return 1;
	}
}
