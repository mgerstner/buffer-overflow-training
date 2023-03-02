Dissecting the exit() System Call
=================================

The program `exit_asm.c` contains inline assembler instructions for setting up
a call to `exit(2)`. This time I've used i686 style system call instructions
for seeing how that works, too. Later we'll use the x86\_64 `syscall` variant.

Can you extract the binary machine code that makes up the exit system call?
There's a hint how to do it in the source code comments.

The program `exit_bin.c` uses the extracted binary machine code in a buffer
and demonstrates how we can (purposely) exchange the return address with the
address of the buffer so we'll execute the exit system call upon return,
instead of actually returning from `main()`.

This way we can test self-contained machine code, whether it works as
expected. We will use these machine code snippets for creating exploit
payloads later on.

Note: You need to install the `execstack` package for successfully building
these example programs.

Exercises
=========

- Compile and run `exit_asm` and observe its exit code to see the system call
  is working as expected.
- Disassemble its main function to see the actual assembler code embedded into
  it.
- Extract the binary machine code representing the system call setup and save
  it in a file. Compare the extracted data with the `exit_code[]` data found
  in `exit_bin.c`.
- Compile and run `exit_bin` and observe its exit code to verify our injected
  machine code is actually executed. You can also use `stepi` in `gdb` to see
  the machine instructions being executed.

Tips and Tricks
===============

- You can understand the binary machine code better when you look up the
  machine code manual. For example [1] for AMD64. The "opcode" is the first
  byte of an instruction that determines the type of instruction and what kind
  of data will follow it. `0xB8` is the opcode for the MOV instruction with
  the source operand being an immediate and the target operand being a
  register. You will find it in the binary machine code for the exit system
  call we use here.

[1]: http://ref.x86asm.net/coder64.html
