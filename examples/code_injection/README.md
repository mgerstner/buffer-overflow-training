Setting Arbitrary Code Execution into Motion
============================================

With the things we've learned from the previous examples we can now make the
final attempt to execute arbitrary code by exploiting a stack buffer overflow.

Once more we have a vulnerable program in `kitty2.c`. We need the piece of
self-contained machine code that executes '/bin/sh' from the previous example
`exec_snippet`. Put this binary code into a separate file.

The accompanying python program `gen_exploit.py` can help you to create the
exploit data we need according to the principles we've learned. It will
prepend NOP instructions, embed the provided machine code and append a number
of return addresses. Inspect the hexdump of one of these exploit sequences to
understand the structure better. An example invocation looks like this:

```
$ ./gen_exploit.py -a 0x7fffa2da0000 -c ./execve.bin -s 256 --output >exploit.bin
$ xxd exploit.bin
```

The still somewhat difficult part now is to find a suitable return address
that will hit our NOP instructions. Otherwise the program will just crash with
SIGILL or SIGSEGV. First you need a shell without 'address space
randomization', which is a security feature to make these kind of overflows
more difficult. You can call `make shell` to get a sub-shell that is configured
to disable this feature.

Then you can try to find out the stack address of the overflowing buffer
either by passing a well defined pattern (see the `zombie_call` example) or by
inspecting the addresses in `gdb`, or by simply printing it from the program
itself. Since we have the NOP slide the buffer address doesn't need to be hit
perfectly. Parametrize the python script using the determined address to
create a suitable exploit buffer.

Save the exploit data in a file and pass the file to the vulnerable program
(not via stdin, but as a parameter). On success a new shell should be started
as a result of the code execution.

NOTE: The reason why feeding the exploit on `stdin` seemingly doesn't work is
that stdin will be in the EOF state and the exploit *will* work but the shell
that is started will detect the closed `stdin` stream and exit immediately.

Exercises
=========

- Experiment with different return addresses and verify that if you hit any of
  the NOP instructions, the exploit will still trigger.
- Dissect the exploit data to understand its structure.
- Manipulate the injected machine code for `execve()` to call a different
  binary or perform a completely different system call (like creating a new
  file).
