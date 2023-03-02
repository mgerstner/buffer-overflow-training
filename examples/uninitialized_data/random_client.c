#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

/*
 * Matthias Gerstner
 * SUSE Linux GmbH
 * matthias.gerstner@suse.com
 */

/*
 * This is a demo client that connects to a server listening on a given UDP
 * port and requests a pseudo random number resulting from ROUND numbers of
 * random() calls on the server side.
 */

/*
 * socket code
 */

int create_and_connect_socket(in_port_t port) {
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

	if (connect(sock, (struct sockaddr*)&local_addr, sizeof(local_addr)) != 0) {
		fprintf(stderr, "Failed to connect socket to port %d: %s\n", port, strerror(errno));
		(void) close(sock);
		return -1;
	}

	return sock;
}

int send_req(int sock, size_t rounds) {
	ssize_t bytes_transferred = 0;

	bytes_transferred = send(
		sock,
		&rounds,
		sizeof(rounds),
		0 /* no flags */
	);

	if (bytes_transferred != sizeof(rounds)) {
		fprintf(stderr, "Failed to send request: %s\n",
				strerror(errno));
		return -1;
	}

	return 0;
}

int recv_reply(int sock, uint64_t *random_nr) {
	ssize_t bytes_transferred = 0;

	bytes_transferred = recv(
		sock,
		random_nr,
		sizeof(*random_nr),
		0
	);

	if (bytes_transferred != sizeof(*random_nr)) {
		fprintf(stderr, "Failed to receive reply: %s\n",
				strerror(errno));
		return -1;
	}

	return 0;
}

int main(const int argc, const char **argv) {
	in_port_t port = 0;
	int socket = -1;
	size_t rounds = 0;
	uint64_t random_nr = 0;

	if (argc != 3) {
		fprintf(stderr, "usage: %s <PORT> <ROUNDS>\n", argv[0]);
		fprintf(stderr, "\nRequests the ROUNDS'th random number "
			"from the random_nr service running on UDP localhost "
			"port <PORT>\n");
		return 1;
	}

	port = parse_integer(argv[1], 10);
	rounds = parse_integer(argv[2], 10);

	socket = create_and_connect_socket(port);

	if (socket == -1)
		return 1;

	if (send_req(socket, rounds) != 0)
		return 1;

	if (recv_reply(socket, &random_nr) != 0)
		return 1;

	printf("Got random nr %lu\n", random_nr);

	(void)close(socket);

	return 0;
}
