Example of an Optimized out memset() Call
=========================================

The stack based buffer `secret` in this program is supposed to be cleared via
`memset()` at the end of the program, but as you can see in the disassembly it
won't be cleared, because it is optimized out by `gcc`.

When built as the alternative target `lost_memset.nobuiltin` then it won't be
optimized out - at least this was the case at the time of writing this, but is
not necessarily true any more.
