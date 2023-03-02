#include <stdio.h>

/*
 * Matthias Gerstner
 * SUSE Linux GmbH
 * matthias.gerstner@suse.com
 */

/*
 * This function finds the location of the return address (i.e. void**) on the
 * stack. `start` needs to be the a 8-byte aligned start address on the stack
 * to start with and `cur_ret` needs to be the current return address.
 *
 * There is no really realiable way to get this address otherwise, since
 * there's usually no need to achieve this programmatically.
 */
void** get_return_address_ptr(void **start, void *cur_ret) {
	int i;

	// We need to reach the return address, it is found some locations
	// after the location of the local stack parameter. this logic will
	// fail if `start` is not on a 8-byte boundary. For safety reasons
	// don't continue until we reach the end of the stack but just look
	// into the first few qwords until we give up.
	for (i = 0; i < 32; i++) {
		start++;

		if (*start == cur_ret)
			return start;
	}

	return NULL;
}

void main() {
	// This builtin gcc function allows to retrieve the current return address value
	void *ret = __builtin_return_address(0);
	void **retp = get_return_address_ptr(&ret, ret);

	/*
	 * this is binary x86_64 code extracted from the exec_asm.c binary that
	 * performs an exec system call for /bin/sh and then, should the exec() fail
	 * for some reason, calls exit(0);
	 */
	const char execve_code[] = {
	  0xeb, 0x31, 0x5b, 0x48, 0x89, 0x5b, 0x08, 0xc6, 0x43, 0x07, 0x00, 0x48,
	  0xc7, 0x43, 0x10, 0x00, 0x00, 0x00, 0x00, 0x48, 0xc7, 0xc0, 0x3b, 0x00,
	  0x00, 0x00, 0x48, 0x89, 0xdf, 0x48, 0x8d, 0x73, 0x08, 0x48, 0x8d, 0x53,
	  0x10, 0x0f, 0x05, 0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00, 0x48, 0x31,
	  0xdb, 0xcd, 0x80, 0xe8, 0xca, 0xff, 0xff, 0xff, 0x2f, 0x62, 0x69, 0x6e,
	  0x2f, 0x73, 0x68
	};

	if (!retp) {
		fprintf(stderr, "Failed to determine return address location!\n");
		return;
	}

	// overwrite the return address to be our injected binary code
	(*retp) = (void*)execve_code;
}

