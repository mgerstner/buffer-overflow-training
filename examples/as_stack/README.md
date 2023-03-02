Implementing a Function in Assembler
====================================

This assembler program is based on the `intro_as` example. The program is
still printing "Hello, world\n" in a loop. But this time the actual print
handling logic has been moved into a reusable function called `print()`.

To implement this `print()` function the stack frame needs to be established
accordingly. Therefore this example allows us to learn how the stack is
typically used.

Building and Running the Program
--------------------------------

The program can simply be built using `make`:

```
$ make
$ ./stack
[...]
```

The Assembler Program Structure
-------------------------------

Look at the differences of this assembler program compared to the `intro_as`
example. Focus on the way the function setup works on both the caller and the
callee side.

The Equivalent C Program
------------------------

The assembler program we use here is roughly equivalent to the following C
program:

```
#include <unistd.h>

void print(const char *s, size_t n) {
	write(STDOUT_FILENO, s, n);
}

const char *msg = "Hello, world\n";

int main() {
    int i = 10;

    do {
        const char *m = msg;
        size_t n = 13;
        print(m, n);
    } while (--i > 0);

    _exit(0);
}
```

Inspecting the Program Flow
---------------------------

In `gdb` you can follow the course of every assembler instruction in this
program. Try to find the return address on the stack when the print() function
is reached.
