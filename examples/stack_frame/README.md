Inspecting the Stack Frame Handling
===================================

This program is not much more than a simple function call with a parameter
passed and some local variables put onto the stack by caller and callee.

Building and Disassembling the Program
--------------------------------------

Build and disassemble this program for a better understanding of the stack
frame logic.

```sh
$ make
$ gdb ./stack_frame

  (gdb) disassemble main
  [...]

  (gdb) disassemble somefunc
```

Make sure no optimization is applied by the compiler and that debug symbols
are added (which the Makefile should care for), otherwise assembler output
will look very differently.

A variation to the above is the `gdb` command `disassemble /s main` which
shows the assembler instruction interleaved with the source code lines they
represent.

Exercise
--------

- Can you find the assembler instruction(s) for saving the `main()` function's
  `%rbp` register on the stack and restoring it before returning to main()?
- Can you find the assembler instruction(s) for saving (and restoring) the CPU
  register for `register int i` on the stack, before it is modified?
- Can you find the assembler instruction(s) for allocating and freeing the
  stack space for the local variables of `somefunc()`?
- Can you find - during runtime - the return address to `main()` on the stack?

Notes
-----

- The `push` instruction without size suffix allocates the canonical register
  size on the stack. This means 4 bytes in 32-bit mode and 8 bytes in 64-bit
  mode. An explicit `pushq` would always push 8 bytes.

### Tips and Tricks

You can start the program in gdb, have a look at the registers and memory and
single step assembler instructions to see how things work:

```
      # start the program
(gdb) start
      # having a look at the current register contents
(gdb) info registers
      # continue until we entered the somefunc() function
(gdb) break somefunc
(gdb) continue
      # this enables assembler view of the program's execution
(gdb) layout asm
      # returns the focus to the prompt
(gdb) <ctrl-x> + o
      # this will single step every machine instruction
(gdb) stepi
      # hexdump of the data on the stack base (remember, the stack grows down!)
(gdb) x/16x $rbp
      # this shows low level context information for the current stack frame
(gdb) info frame
      # this shows all register contents
(gdb) info registers
      # an alternative is 'layout regs' to permanently display register state
(gdb) layout regs
```
