/*
 * Matthias Gerstner
 * SUSE Linux GmbH
 * matthias.gerstner@suse.com
 */

/*
 * This program sets up an execve() system call to run /bin/sh with empty
 * environment and no extra parameters. This is done using inline assembly and
 * the x86_64 syscall instruction.
 */

/*
 *  This program modifies the .text segment (i.e. the code modifies itself)
 *  which is not allowed by default.
 *
 *  This happens only for demonstration purposes to test the assembler code so
 *  bypassing this restriction is not really relevant for the security exploit
 *  in practice.
 *
 *  To get this code to work for testing we can modify the page protection
 *  during runtime with mprotect() or link with writable text segments.
 *
 *  The ld option -N recently started producing a linker error with
 *  __ehdr_start being undefined. It seems to have to do with the GOT/PLT
 *  table being generated and requiring dynamic linking. Therefore we use the
 *  mprotect approach now.
 */
#define USE_MPROTECT

#ifdef USE_MPROTECT

#	include <sys/mman.h>
#	include <stdint.h>

static void make_text_writable(void *mainptr) {
	// mark main() code writable during runtime, an alternative to linking
	// with -Wl-N and statically
	intptr_t addr = (intptr_t)mainptr;
	// round down to the page size boundary
	addr = addr & (~(intptr_t)(4096-1));
	// allow all operations on the code
	mprotect((void*)addr, 4096, PROT_READ | PROT_WRITE | PROT_EXEC);
}

#endif // USE_MPROTECT

int main() {
#ifdef USE_MPROTECT
	make_text_writable(&main);
#endif

	__asm__(
#include "simple.S"
	);
}

