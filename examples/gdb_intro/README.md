Example Program for Testing `gdb` Invocation and First Simple `gdb` Steps
=========================================================================

Prerequisites
-------------

### Installing `debuginfo` Packages

You will need to install debugging symbols for the `glibc`. It may be
necessary to enable `Debug` and `Sources` repositories in `zypper` before this
works. See `zypper lr -d`. When the repositories are available, the required
packages can be installed as *root* (or via `sudo`) like this:

    root# zypper in glibc-debuginfo

### Using `debuginfod` Instead

Alternatively, if on openSUSE Tumbleweed, you can use the `debuginfod` feature
which will allow you to transparently download debug symbols for system
packages. In this case you should mark the use of this feature permanent by
adding

    set debuginfod enabled on

to your `~/.config/gdb/gdbinit`, which might first need to be created.
Similarly, if you don't want to use this feature, you can permanently
configure it as `off` to avoid `gdb` asking you whether to contact network
servers all the time.

Compile and Run the Program
---------------------------

A `gdbtest` program exists in this directory that can be built using `make`.
Look at the C program, understand what it does and observe its behaviour.

    $ make
    $ ./gdbtest
    [...]
    $ ./gdbtest 5
    [...]

Custom CFLAGS can be passed to the `make` program via the `CFLAGS` environment
variable. By default the `-g` switch is passed to `gcc` to generate debug
symbols. You can clear the CFLAGS by running `make` like this:

    $ make clean; CFLAGS= make
    [...]

This will generate a binary without debugging information. You can observe the
different behaviour of `gdb` with the two versions.

Invoke the Program in `gdb`
---------------------------

There are various possibilities to do this:

1. Simply point `gdb` to the program: `gdb ./gdbtest`.
2. Pass parameters directly on the command line: `gdb --args ./gdbtest somepar`.
3. Attach to a running program: `gdb -p <PID>`.

In all cases you will enter the `gdb` shell shown by the prompt `(gdb)`. The
`gdb` shell accepts a lot of different commands. We will learn some of the
basic ones during this training.

By typing `start` the program will start, but stop right away in the `main()`
function. If all went well then you should be able to see the first source
line in the `main()` function and to inspect the running program.

NOTE: In recent openSUSE Tumbleweed installations attaching even to your own
processes is not allowed by default anymore, for reasons of security
hardening. To overcome this you can either:

- install the package `aaa_base-yama-enable-ptrace`. This will permanently
  disable the security feature (should only be done on test or dedicated
  development systems).
- execute `sysctl kernel.yama.ptrace_scope=0` to disable the hardening until
  the next reboot.

Experimenting with `gdb`
------------------------

You can find more information on basic commands in the slides following this
example. Experiment with them to get a feeling for how `gdb` works.

Running a System Program in `gdb`
---------------------------------

Let's try to debug the `ls` program.

When using the debuginfo package then we first we need to find out which
package it belongs to:

    $ rpm -qf /usr/bin/ls
    coreutils-8.31-2.1.x86_64

Then install the according `debuginfo` and `debugsource` package.

    root# zypper in coreutils-debuginfo coreutils-debugsource

Look at the contents of these packages to better understand their structure:

    $ rpm -ql coreutils-debuginfo
    [...]
    $ rpm -ql coreutils-debugsource
    [...]

After doing this you should be able to successfully debug the `ls` command.
For example:

    $ gdb --args /usr/bin/ls /tmp
    (gdb) start
    [...]

---
**NOTE:**

`gdb` might still complain about some missing debuginfo packages of
third-party libraries that `ls` uses. This normally still allows you to debug
the main program, although you won't be able to step into functions that
belong to these third-party libraries.

---
