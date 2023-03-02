#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "common.h"

/*
 * Matthias Gerstner
 * SUSE Linux GmbH
 * matthias.gerstner@suse.com
 */

/*
 * This is a demo server listening on a given UDP port for requests to return
 * a pseudo random number. Clients can pass a <rounds> parameter that
 * determines the number of random() calls the server should perform before
 * returning the result.
 */

static const size_t MAX_ROUNDS = 64;
static int initialized_random = 0;

/*
 * random generator code
 */

bool read_real_random(char *to, size_t bytes) {
	ssize_t all_read = 0;
	ssize_t just_read = 0;
	/* read some high quality random data from /dev/random */
	int fd = open("/dev/random", O_RDONLY | O_CLOEXEC);

	if (fd == -1) {
		fprintf(stderr, "Failed to open random device: %s\n",
				strerror(errno));
		return false;
	}

	while (all_read != bytes) {
		just_read = read(fd, to + all_read, bytes - all_read);

		if (just_read == -1) {
			fprintf(stderr, "Failed to read from random device: %s\n",
					strerror(errno));
			return false;
		}

		all_read += just_read;
	}

	(void) close(fd);
	return true;
}

bool init_random() {
	unsigned int seed = 0;
	bool ret;

	ret = read_real_random((char*)&seed, sizeof(seed));
	printf("Using RNG seed %u\n", (unsigned int)seed);

	srand(seed);
	return ret;
}

uint64_t gen_random(size_t rounds) {
	uint64_t ret;

	for (size_t round = 0; round < rounds; round++) {
		ret = (uint64_t)random();
	}

	return ret;
}

/*
 * socket code
 */

int create_and_bind_socket(in_port_t port) {
	/*
	 * use an UDP socket on a local port for receiving requests for random
	 * numbers
	 */
	const struct sockaddr_in local_addr = {
		AF_INET,
		port,
		{ htonl(INADDR_LOOPBACK) }
	};
	int sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);

	if (sock == -1) {
		fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
		return sock;
	}

	if (bind(sock, (struct sockaddr*)&local_addr, sizeof(local_addr)) != 0) {
		fprintf(stderr, "Failed to bind socket to port %d: %s\n", port, strerror(errno));
		(void) close(sock);
		return -1;
	}

	return sock;
}

struct request {
	struct sockaddr_in requestor;
	size_t rounds;
	uint64_t reply;
};

int get_next_req(int sock, struct request *req) {
	ssize_t bytes_recvd = 0;
	socklen_t addrlen = sizeof(req->requestor);

	memset(req, 0, sizeof(*req));

	bytes_recvd = recvfrom(
		sock,
		&(req->rounds),
		sizeof(req->rounds),
		0 /* no flags */,
		(struct sockaddr*)&(req->requestor),
		&addrlen
	);

	if (bytes_recvd == -1 || addrlen != sizeof(req->requestor)) {
		fprintf(stderr, "Failed to receive request: %s\n", strerror(errno));
		return -1;
	} else if (bytes_recvd != sizeof(req->rounds)) {
		fprintf(stderr, "Short read of %ld bytes instead of %ld bytes\n",
			bytes_recvd, sizeof(req->rounds)
		);
		return -1;
	}

	return 0;
}

int reply_to_req(int sock, const struct request *req) {
	ssize_t sent_bytes = sendto(
		sock,
		&(req->reply),
		sizeof(req->reply),
		0 /* no flags */,
		(struct sockaddr*)&(req->requestor),
		sizeof(req->requestor)
	);

	if (sent_bytes != sizeof(req->reply)) {
		printf("Failed to reply to request: %ld %s\n", sent_bytes, strerror(errno));
		return -1;
	}

	return 0;
}

/*
 * main
 */

int main(const int argc, const char **argv) {
	in_port_t port = 0;
	int socket = -1;
	struct request req;

	if (argc != 2) {
		printf("usage: %s <PORT>\n", argv[0]);
		printf("\nListens on the given UDP port for requests for random numbers\n");
		return 1;
	}

	port = parse_integer(argv[1], 10);

	socket = create_and_bind_socket(port);

	if (socket == -1) {
		return 1;
	}

	/* service any clients */
	while (1) {
		if (get_next_req(socket, &req) != 0)
			break;

		if (req.rounds > MAX_ROUNDS) {
			printf("Requested %lu rounds exceeds limit, reducing to %lu rounds\n",
				req.rounds,
				MAX_ROUNDS
			);

			req.rounds = MAX_ROUNDS;
		}

		if (!initialized_random) {
			if (!init_random())
				break;
			initialized_random = 1;
		}

		req.reply = gen_random(req.rounds);

		if (reply_to_req(socket, &req) != 0)
			break;
	}

	(void) close(socket);

	return 1;
}
