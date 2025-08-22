Stack Buffer Overflow in editorconfig-core-c in `ec_glob()`
===========================================================

This vulnerability was handled in SUSE bugzilla as [bsc#1233815][1] and was
assigned CVE-2024-53849. The upstream Git repository is found on [GitHub][2].
The issue was fixed in version 0.12.7. For inspecting the vulnerability we
need to `git checkout v0.12.6`.

[1]: https://bugzilla.suse.com/show_bug.cgi?id=1233815
[2]: https://github.com/editorconfig/editorconfig-core-c.git

About editorconfig
------------------

editorconfig is a software suite for applying editor configuration in an
application independent way. A central configuration file `.editorconfig` can
be placed e.g. in a Git repository. Editors with editorconfig support
(sometimes provided by additional plugins) will lookup and parse this
configuration file and apply editor settings accordingly, when opening source
code files in a project.

editorconfig-core-c is a library implemented in C, which can be used by
editors or plugins to link against, for parsing `.editorconfig` files.

The Vulnerability
-----------------

The issue is found in function `ec_glob()` in `src/lib/ec_glob.c`. To better
understand what is happening here it is useful to understand what the code is
trying to achieve. The man page `man 5 editorconfig-format` tells us:

> EditorConfig files use an INI format that is compatible with the format used
> by Python ConfigParser Library, but [ and ] are allowed in the section
> names. The section names are filepath globs, similar to the format accepted
> by gitignore.

This means the usually fixed-string INI section names like `[general]` undergo
special parsing in the editorconfig library. The logic starts in the exported
library function `editorconfig_parse()`, which receives the path of a file to
be edited, i.e. not the `.editorconfig` file, the function rather tries to
look up a matching `.editorconfig` file for the given path.

For each `.editorconfig` candidate the function comes up with, the function
`ini_parse()` is called (`editorconfig.c:531`). `ini_parse()` tries to open
the path and, if it succeeds, passes control on to `ini_parse_file()`. This is
the place where `.editorconfig` is actually parsed line by line. As soon as a
`name=value` pair is encountered by the code, a callback `ini_handler()` is
invoked. This callback receives the INI section name that was last seen.

The `ini_handler()` allocates a buffer on the heap according to rather complex
calculations in `editorconfig.c:239`. This buffer serves for expansion special
syntax in the section name. Some of this expansion is already done in the
handler function itself (by adding the parent directory as a prefix), but the
main business logic is then found in the function `ec_glob()`, which is passed
the heap-allocated buffer containing the pre-processed INI section name.

`ec_glob()` parses the glob pattern times in complex ways with the goal of
turning the glob pattern into a PCRE2 regular expression. A for loop of over
200 lines in length starting at `ec_glob.c:134` is where the vulnerability is
finally found. The function places a buffer `char pcre_str[2*PATTERN_MAX]`
onto the stack. This is where the PCRE regular expression will be constructed
piece by piece. `PATTERN_MAX` is 4097 bytes, thus the stack buffer here is
about 8 KiB in size. The pointer `p_pcre` points to the current position in
this buffer. The pointer `pcre_str_end` points to the end of this buffer, it
is used for determining out-of-bound access attempts in various macros invoked
in this function.

There is one code path which completely ignores these safeguards, however, and
this is in `case '['`, when the code tries to literally extract a substring
consisting of `[.*/.*]`. If an opening square bracket is found and also a
slash is following in the section name, then everything up to a closing square
bracket or the end of the section name string is copied into `p_pcre` via an
`strncat` in `ec_glob.c:196`. The buffer output buffer limits are not checked
for in this case.

Triggering the Vulnerability
----------------------------

First we need to build a vulnerable version of the editorconfig library. I did
this the following way:

```
$ git clone https://github.com/editorconfig/editorconfig-core-c.git
$ cd editorconfig-core-c
$ git checkout v0.12.6
# you may need to install cmake first
$ sudo zypper in cmake
# we also need libpcre2 devel files
$ sudo zypper in pcre2-devel
$ cmake -DCMAKE_C_FLAGS="-fsanitize=address -O2 -g" .
$ make
```

You can find an example `.editorconfig` file which triggers the overflow in
[overflow-input.txt][3]. A copy of the file is also found in this example
folder. The editorconfig package also contains an `editorconfig` program. The
exact purpose of the program is not fully clear to me from the documentation.
It can be used for triggering the exploit, however, because it invokes the
vulnerable code in the editorconfig-core-c library.

[3]: https://github.com/editorconfig/editorconfig-core-c/files/14320240/overflow-input.txt

The following invocation should trigger the address sanitation logic
(without the `-fsanitize=address` compilation flag the error might not be
visible).

```
# copy over the example configuration file into an active .editorconfig
$ mkdir test-folder
$ cd test-folder
$ cp /path/to/overflow-input.txt .editorconfig

# the tool expects an absolute path with a trailing / to look for a file in
# the current directory
$ ./bin/editorconfig $PWD/
=================================================================
==2666==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x7b8558c02082 at pc 0x7f855af1aead bp 0x7ffead915730 sp 0x7ffead914ef0
```

Starting from here you can now experiment with different builds (e.g. without
`-fsanitize=address`) and inspect the situation in `ec_glob()`, feeding
different inputs to it.

Properties of the Stack Overflow
--------------------------------

There are several limitations to the input bytes for this stack buffer
overflow. To start with, the parser operates line-based, i.e. the exploit
cannot contain newlines. Furthermore some characters like `;` and `#` are
treated specially. The `strncat` only copies data up to an `]` or `\0`
character, thus these bytes also cannot appear in the payload.

### Optimized Build

In my tests, when using an optimized build (compile with `-O2` flag), the
vulnerable `pcre_str` buffer of 8 KiB size is located 16,472 bytes from the
`rip` pointer saved on the stack. This means for a successful exploit we need
to be able to overflow more than 16 KiB onto the stack. There is an obstacle
to this: the input payload can be at most 4096 bytes long (see `MAX_SECTION`
preprocessor constant, used in `ini_parse_file()`). There are various
curiosities in the wildcard parsing code in `ec_glob()`, however, that allow
us to expand the input buffer along the way. For example each `?` in the input
will cause four characters to be written into the output buffer. Each `*`
which is not followed by another `*` will cause five input characters to be
written into the output buffer.

While a combination of these characters would be enough to jump the gap of
16,472 bytes to reach the RIP on the stack, these expansions are written to
the output buffer using the `STRING_CAT` macro, which enforces buffer end
checks. Thus we can only fill up the 8 KiB of the `pcre_str` buffer using this
code path. To reach this point we need to spend about 1,800 bytes of our
exploit buffer, a sequence of `?*` bytes, which will fill up `4 * 900 + 6 *
900 = 8,100 bytes`. Now we have only about 2,300 bytes left in the exploit
buffer. Using the unbounded `strncat` code path we can overflow them past the
end of the `pcre_str` buffer, but this only reaches a position some 10,500
bytes beyond the `pcre_str` buffer. This means we cannot reach the `rip`
stored on the stack.

Interestingly, the reason why the RIP is placed so far away from the
`pcre_str` buffer is that a copy of the input pattern (the exploit data) is
placed on the stack as well in `char l_pattern[2 * PATTERN_MAX]`. In the
optimized build this buffer is actually placed _after_ the `pcre_str` buffer,
increasing the distance to the saved RIP register value by 8 KiB. This also
means that, by overflowing past the `pcre_str` buffer, we can overwrite our
own input pattern to some degree! This results in a very complex machinery
which makes it hard to tell if a very tricky exploit buffer might still yield
success.

### Debug Build

When using a debug build (compiled with `-O0` flag) then, in my case, the gap
between `pcre_str` and the `rip` pointer is only a bit above 8 KiB. In this
scenario it is possible to overflow the `rip` stored on the stack. There is
another obstacle, however: We cannot overflow `\0` bytes over `rip`. On 64-bit
we need to achieve, this, however, to place a valid return address into `rip`.

We might be able to achieve code execution by hitting the `rip` on the stack
perfectly, taking advantage of the leading zero bytes already present for the
legitimate `rip`. We must not write even one additional byte, however, to
succeed.

I tested this approach but failed in the end for other reasons: In the debug
build most of the variables placed on the stack are indeed on the stack, and
not moved into registers. When overflowing past the `pcre_str` buffer, we will
therefore corrupt many pointers also present on the stack. Including
`pcre_str_end`, by the way! This means we might even be able to disarm the
existing `STRING_CAT` macros this way. It is difficult, while not impossible,
to come up with a working exploit that keeps all the necessary pointers on the
stack intact, while skillfully overwriting the RIP pointer on the stack with a
new address. I did not manage to fully complete this approach, however.

The script `build_overflow.py` found in the example directory is a first
attempt to construct a suitable attack payload. You can experiment a bit with
this and see what happens in the code.

Possible Attack Scenario
------------------------

This example shows how a feature, which seems to be a concern of local
security only on first sight, can easily become a remote attack scenario.
Since various different editor programs might lookup `.editorconfig` files,
maybe even without the user knowing it, it is enough to open a source code
file in a cloned Git repository to trigger the vulnerability. The victim
doesn't even need to open the file containing the exploit, which makes this
well hidden, unless a crash occurs.

One of the editors which I found to integrate editorconfig-core-c is KDE's
`kate`, or more generally, the ktexteditor framework from KDE. Any KDE
application that uses ktexteditor which operates on a checkout containing an
`.editorconfig` file is susceptible to an attack.

A test of this works as follows:

```sh
# install kate, if not already present
$ sudo zypper in kate
# create a debug build of editorconfig without sanitizer
# this is necessary to make the test work and to cause a visible crash
$ make clean
$ cmake -DCMAKE_C_FLAGS="-g"
$ make
# enter the directory containing the compiled, vunlerable editorconfig libs
$ cd editorconfig-c-core/lib
# configure the dynamic linker to prefer libraries found in this directory
$ export LD_LIBRARY_PATH=$PWD
# now run the editor and open a file in a directory containing a crafted .editorconfig
$ kate
```

Lessons Learnt
--------------

This case study provides various lessons for our training:

- even seemingly safe functions like `strncat()` can be used in unsafe ways.
- the logic to parse INI files in the library is convoluted and spread
  throughout the code:
  - the use of macros makes some things intransparent.
  - the calculation of the output buffer size is only a rough estimate and not
    consistent on all levels of the source code.
  - the same data is found in multiple buffers managed by different functions
    (heap buffers, stack buffers, input pattern, expanded pattern).
  - we have to deal with multiple pointers to the same data in `ec_glob()`
    (`pcre_str`, `pcre_str_end`, `p_pcre`, `c`, `cc`, `l_pattern`).
  - the conversion of a wildcard pattern into a PCRE expression is highly
    complex and would need triple care to avoid security issues.
  Such parsing logic can be hard to develop in a digestible way, especially in
  plain C. I believe it still needs to be better than what we can find here at
  the moment.
- from looking into various editorconfig plugins I found that there exist many
  independent implementations for parsing `.editorconfig` files. This means
  that likely not all features will be supported by all plugins, also different
  types of bugs will be found in them.

Further Resources
-----------------

For a detailed analysis of previous issue, providing even more degrees of
freedom in the same code, check out [this report][4].

[4]: https://litios.github.io/2023/01/14/CVE-2023-0341.html
