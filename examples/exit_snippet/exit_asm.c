/*
 * Matthias Gerstner
 * SUSE Linux GmbH
 * matthias.gerstner@suse.com
 *
 * This program exits with an exit code of 5 by issuing the exit() system call
 * via inline assembly.
 *
 * You can extract the assembly code via gdb like this:
 *
 * ```
 *       # inspect assembler code locations and select the desired range
 * (gdb) disas main
 * [...]
 *       # write binary data starting from main+X till location main+Y into out.bin,
 *       # where X and Y are the offsets of the code you would like to extract
 * (gdb) dump memory out.bin main+X main+Y
 * ```
 *
 * the resulting file can be formatted using 'xxd -i' to obtain a suitable C
 * string literal for further usage.
 */

int main() {
	// For completeness this exit system call uses the 32-bit
	// i686 calling convention for Linux.
	// This only works for register values <= 32 bit, higher bits will be
	// zeroed out. This approach will also be slower than the AMD64
	// syscall instruction (when running on x86_64 hardware, of course).
	__asm__(
		"movl $0x1, %eax\n"
		"movl $0x5, %ebx\n"
		"int $0x80"
	);
}
