Dissecting the execve() System Call
===================================

Similar to what we did in the previous example `exit_snippet` for the
`exit()` system call we now want to extract a piece of machine code that
executes the `execve()` system call with parameters set for starting `/bin/sh`
i.e. a new shell.

Because we need to provide pointers to strings and arrays of strings the setup
of self contained code is not so easy this time. We are triggering the system
call using the `x86_64` `syscall` instruction and register sets this time.

The assembler code also embeds a string for '/bin/sh'. But we won't know at
which address this string will be located during runtime. The current
instruction pointer from register `rip` also cannot be directly accessed due
to limitations of the instruction set.

We are exploiting the `jmp` and `call` instructions to overcome this
limitation. `jmp` allows to specify a relative address where to continue
code execution. The same is true for the `call` instruction, but interestingly
it also pushes the return address of the next instruction on the stack. The
`call` instruction is the sibbling of the `ret` instruction to help
implementing function calls. By combining `jmp` and `call` we can obtain the
memory address of the '/bin/sh' string during runtime in a reliable way,
without knowing any absolute addresses in advance.

Simlar to the `exit_snippet` example we have a file `exec_asm.c` which
contains inline assembler doing what we want. The extracted raw machine
instructions making up the self contained `execve()` code for '/bin/sh' is
then found in `exec_bin.c` for testing.

Because this code is self modifiying we can't by default run it in the version
contained in `exec_asm.c`, because the text segment is by default read-only.
The `Makefile` contains directives for making the .text segment writeable to
overcome this security feature for testing purposes.

Exercises
=========

- Try to read the assembler code carefully for understanding what is going on
  here.
- You can extract the machine code from `exec_asm` similar to what we did in
  the `exit_snippet` example.
- Run the `exec_bin.c` program and step until the syscall assembler
  instruction is about to be executed. Inspect the register values and memory
  locations we're pointing to for seeing the result of the code in action.
