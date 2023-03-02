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
	 * This is binary machine code extracted from the binary for exit_asm.c that
	 * performs an exit system call with status code 5
	 */
	const char exit_code[] = {
		0xb8, 0x01, 0x00, 0x00, 0x00, 0xbb, 0x05, 0x00, 0x00, 0x00, 0xcd, 0x80
	};

	if (!retp) {
		fprintf(stderr, "Failed to determine return address location!\n");
		return;
	}

	// Overwrite the return address to point to our injected binary code
	(*retp) = (void*)exit_code;

	// Upon returning, the machine code to call exit(5) should be executed
	// instead of the regular return. This can be verified by inspecting
	// the program's return code which should be 5 instead of 0.
}
