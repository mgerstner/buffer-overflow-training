	#
	# Matthias Gerstner
	# SUSE Linux GmbH
	# matthias.gerstner@suse.com
	#
	# this declares the start code label where execution begins when
	# the program is started by the kernel.
	.global _start

	# this declares a code section
	.text
 
	# entry point for a print(const char*, size_t) function
	#
	# purpose: print a string to stdout
	#
	# expects two parameters passed on the stack:
	# - (const char*) pointer: string data to print
	# - (uint64_t): 64-bit unsigned integer: length of string data, excluding '\0' terminator
print:
	push	%rbp		# save the previous stack base pointer on the stack
	mov	%rsp, %rbp	# the current top of the stack is where our own stack frame for this function starts
	# save the contents of registers we want to use on the stack, since we want to use them for our own purposes
	push	%rax
	push	%rdx
	push	%rdi
	push	%rsi
	# setup the write(int fd, const void*, size_t) system call
	mov	$1, %rax	# system call 1 is write
	mov	$1, %rdi	# parameter 1: file handle 1 is stdout
	mov	0x18(%rbp),%rsi # place the first input parameter (pointer to the string to print) into %rsi
	mov	0x10(%rbp),%rdx # place the second input parameter (the string length) into %rdx
	syscall			# request the operating system to perform the configured syscall

	# restore original register contents by popping data back from the stack in reverse
	pop	%rsi
	pop	%rdi
	pop	%rdx
	pop	%rax
	pop	%rbp
	# return to caller (uses return address stored on stack by `callq`)
	retq

	# entry point for the "main()" function
_start:
	mov	$10, %rax	# store the loop counter 10 in rax
loop_start:
	sub	$0x10,%rsp	# allocate 16 bytes on stack for passing parameters to print()
	movq	$message, 0x08(%rsp) # store the pointer to "Hello, world\n" on the stack as first parameter to print()
	movq	$13, (%rsp)	# store the immediate value 13 as size specification for the second parameter to print()
	callq	print		# stores the return address on the stack and jumps to the print() function
	add	$0x10,%rsp	# free 16 bytes to remove parameters to print() again from stack

	# loop handling
	dec	%rax		# decrement the loop counter
	jnz	loop_start	# jump-not-zero: perform another loop operation
				# if the counter hasn't hit zero yet.

				# the dec instruction stores a flag that
				# indicates zero/non-zero result

	# exit(0)
	mov	$60, %rax	# system call 60 is exit
	xor	%rdi, %rdi	# we want return code 0, this is a cheap instruction to generate a 0 in register rdi
	syscall			# invoke operating system to exit
message:
	.ascii	"Hello, world\n" # this declares a program constant, ASCII text in this case

# vim: set ts=8:
