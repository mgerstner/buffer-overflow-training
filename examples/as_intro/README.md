Hello World Assembler Program
=============================

In `intro.s` you will find an example assembler program that will print out
"hello world" to stdout. The `.s` extension is conventionally used for
assembler code.

Building and Running the Program
--------------------------------

The program can simply be built using `make`:

```
$ make
$ ./intro
[...]
```

In the Makefile you can see how the program is built. Calling the GNU
assembler `as` directly only creates an object file. It still needs to be
linked by calling the linker `ld` explicitly on the resulting object file.

The `gcc` compiler itself is only generating assembler code on-the-fly which
is transparently passed to the assembler, to generate machine code.

The Assembler Program Structure
-------------------------------

Look at the assembler source and try to understand its structure. It consists
of different sections for declarations, code and data. Jumps (like a `goto` in
C) are the basic mechanism of flow control. To actually make an assembler
program human readable a lot of comments are needed, because you can't just
declare a variable by name like in higher programming languages. All you have
are labels for performing jumps and for accessing constant data.

The Equivalent C Program
------------------------

The assembler program we see here is roughly equivalent to the following C
program:

```
#include <unistd.h>

int main() {
    int i = 10;

    do {
        write(STDOUT_FILENO, "Hello, world\n", 13);
    } while (--i > 0);

    _exit(0);
}
```

It is clearly visible that the C program is shorter and easier to understand.

You can also try to see what kind of assembler output the compiler would
generate for such a program by storing the above program in a file `intro.c`
and calling `gcc -S intro.c -ogcc_intro.s`. The generated assembler code will
be more difficult to understand though, since the compiler is using all kinds
of expert knowledge to organize the assembler code.

Experimenting with the Assembler
--------------------------------

You can run the program in `gdb` and inspect the various registers while
running. There is a special command to deal with Assembler in `gdb`:
`stepi`. This command will continue program execution for exactly one
processor instruction. You will also need to set a special breakpoint, because
our assembler program does not conform to usual program conventions, that are
followed when a program is compiled by `gcc`:

```
gdb ./intro
[...]
(gdb) b _start
(gdb) r
Breakpoint 1, _start () at intro.s:7
(gdb) stepi
[...]
(gdb) info registers
```

Also try to understand the purpose of the `eip` or `rip` register as the
instructions progress.
