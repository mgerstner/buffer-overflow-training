Training Material "Anatomy of Buffer Overflows and Low Level Security"
======================================================================

Overview
--------

This repository contains presentation slides and training examples for
learning about exploitation of stack buffer overflows on Linux systems. The
target audience is beginners with existing basic knowledge about C programming
and Linux. The training covers the following topics:

- Using `gdb` for debugging, inspecting buffer overflows during runtime and
  interacting with the program also on assembler level.
- A basic introduction to assembler programming on `i386` / `x86_64`
  processors.
- Explanation of a computer's address space, the function and management of
  stack and heap memory, how function calls are setup, how system calls are
  setup and related topics.
- Typical techniques to exploit a stack buffer overflow with the aim of
  arbitrary code execution. Various examples of growing complexity help to get
  in touch with the topic step by step.
- Modern protection measures to prevent stack buffer exploits are discussed.

The examples are all tailored towards running them on current openSUSE Linux
distributions.

The material is used for a 3 to 4 day training for trainees in computer
science. Due to the advanced nature of the topics also more experienced
engineers that aren't familiar with low level programming and buffer overflows
can profit at least from parts of the training.

How to Build
------------

This presentation is based on `asciidoc`. Installing it should be enough to
successfully run `make`. The result will be a single HTML file containing also
embedded images. The presentation can be opened in a regular web browser.

Licensing
---------

The content of this repository (the presentation slides, PNG images and their
SVG sources as well as all example code and its documentation) are available
under the Creative Commons license BY-NC-SA 4.0. See the `LICENSE` file in the
repository or [online][1].

[1]: https://creativecommons.org/licenses/by-nc-sa/4.0

This basically means you can use it, share it, adapt it for non-commercial
uses if you mention the original author and source and grant the same rights
defined in the license to others.
