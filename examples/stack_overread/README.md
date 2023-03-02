Stack Buffer Overread
=====================

This program has a stack buffer overread problem. It is caused by missing
zero-initializion and not communicating back the number of bytes actually
read.

Exercises
=========

- Find the overread issue and trigger it during runtime
- What kind of information can you gain from this weakness?
- How could some of this information help an attacker when combined with other
  weaknesses in a program?
