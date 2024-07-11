Stack Buffer Overread
=====================

This program has a stack buffer overread problem. It is caused by missing
zero-initialization and not communicating back the number of bytes actually
read.

NOTE: Technically this program does not _overread_ a stack buffer, but it
reads from an uninitialized stack buffer. The effects are very similar,
though.

Exercises
=========

- Find the overread issue and trigger it during runtime
- What kind of information can you gain from this weakness?
- How could some of this information help an attacker when combined with other
  weaknesses in a program?
