#include <unistd.h>

/*
 * Matthias Gerstner
 * SUSE Linux GmbH
 * matthias.gerstner@suse.com
 */

#ifndef REAL_USER_ID
#	error "you need to define REAL_USER_ID to the value of `id -u`"
#endif

uid_t geteuid() {
	return REAL_USER_ID;
}

uid_t getuid() {
	return REAL_USER_ID;
}
