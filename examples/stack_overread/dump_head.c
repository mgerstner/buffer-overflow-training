#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Matthias Gerstner
 * SUSE Linux GmbH
 * matthias.gerstner@suse.com
 */

/*
 * This program outputs a hexdump of the first N bytes from a user supplied file.
 */

#define MAX_LENGTH 4096
#define TMP_BUF_SIZE 128

void readfile(int fd, char *buf, size_t bufsize) {
	char tmp[TMP_BUF_SIZE];
	size_t read_so_far = 0;
	ssize_t read_res = -1;

	while (read_so_far < TMP_BUF_SIZE && read_so_far < bufsize) {
		read_res = read(fd, tmp + read_so_far, TMP_BUF_SIZE - read_so_far);

		if (read_res == 0) {
			// EOF
			break;
		} else if (read_res == -1) {
			fprintf(stderr, "Failed to read from file: %s\n", strerror(errno));
			return;
		}

		read_so_far += read_res;
	}

	memcpy(buf, tmp, bufsize);
}

void hexdump(const char *buf, unsigned long count) {
	unsigned long byte;

	for (byte = 0; byte < count; byte++) {
		if ((byte % 32) == 0) {
			printf("\n");
		} else if( (byte % 4) == 0) {
			printf(" ");
		}

		printf("%02x", (unsigned char)buf[byte]);
	}

	printf("\n");
}

int main(const int argc, const char **argv) {
	int fd = -1;
	unsigned long count = 0;
	char *printbuf = NULL;
	const char *path = argv[1];
	char *endptr = NULL;

	if (argc != 3) {
		printf("usage: %s <PATH> <LENGTH>\n\n", argv[0]);
		printf("This program will read LENGTH bytes from PATH and output them to stdout as a hexadecimal dump.\n");
		return 1;
	}

	count = strtoul(argv[2], &endptr, 10);

	if ((count == ULONG_MAX && errno == ERANGE) ||
			count > MAX_LENGTH || endptr == argv[2]) {
		printf("LENGTH couldn't be parsed or is out of range.\n");
		return 1;
	}

	fd = open(path, O_RDONLY | O_CLOEXEC);

	if (fd == -1) {
		printf("Failed to open %s: %s\n", path, strerror(errno));
		return 1;
	}

	printbuf = malloc(count);

	readfile(fd, printbuf, count);

	printf("Hexdump of %ld bytes of %s\n", count, path);
	hexdump(printbuf, count);

	(void)close(fd);
	free(printbuf);

	return 0;
}
