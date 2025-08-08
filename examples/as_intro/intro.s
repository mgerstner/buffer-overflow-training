	.global _start		# this declares the start code label where execution
				# begins when the program is started by the kernel.

	.text			# this declares a code section
_start:
	mov	$10, %rbx	# store the loop counter 10 in rbx
loop_start:
	# write(1, message, 13)
	mov	$1, %rax	# system call 1 is write
	mov	$1, %rdi	# parameter 1: file descriptor 1 is stdout
	mov	$message, %rsi	# parameter 2: address of string to output
	mov	$13, %rdx	# parameter 3: number of bytes in string
	syscall			# invoke operating system to perform the syscall

	# loop handling
	dec	%rbx		# decrement the loop counter
	jnz	loop_start	# jump-not-zero: perform another loop iteration
				# if the counter hasn't hit zero yet.

				# the dec instruction sets a flag in the CPU
				# which indicates zero/non-zero result

	# exit(0)
	mov	$60, %rax	# system call 60 is exit
	xor	%rdi, %rdi	# we want return code 0, this is a cheap instruction to generate zero in register rdi
	syscall			# invoke operating system to exit
message:
	# this declares a program constant, ASCII text in this case
	.ascii	"Hello, world\n"

# vim: set ts=8:
