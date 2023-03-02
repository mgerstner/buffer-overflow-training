Returning to a Different Function than Intended
===============================================

This program also contains a typical buffer overflow vulnerability like the
previous example did. This time we want to replace the return address such
that the function `zombie()` is called upon return.

Exercises
=========

- Experiment with different overflow patterns and see what happens during
  program execution.
- What could be a way to determine exactly where the new return address needs
  to go in the overflow data?
- The accompanying python program `call_zombie.py` automates some of the tasks
  like determining the address of `zombie()` in the program, creating a
  suitable exploit payload and feeding it to the vulnerable program. You can
  experiment with different parameters.
