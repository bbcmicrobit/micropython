#!/usr/bin/env python3
'''
Produce a combined HEX file.
'''
import os
import sys
import argparse

from intelhex import IntelHex

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Create a hex version of a Python script')

    parser.add_argument('pythonfile', type=argparse.FileType('rb'), help='Python file to be hexlified')

    # this is applicable to hexlify and merge
    parser.add_argument('-o', '--output', type=argparse.FileType('wb'), help='Output hex file (default: stdout)', default=sys.stdout)
    parser.add_argument('-a', '--address', type=int, help='Start address for the python script', default=0x3e000)

    # this is only useful if you're merging.
    parser.add_argument('-m', '--merge', nargs='?', type=argparse.FileType('rb'), help='micropython hex file to merge with', default=os.devnull)

    args = parser.parse_args()

    base_hex = IntelHex(args.merge)
    python_hex = IntelHex()

    data = bytes(args.pythonfile.read())
    data = b'MP' + len(data).to_bytes(2, 'little') + data + bytes(16 - len(data) % 16)
    python_hex.puts(args.address, data)

    base_hex.merge(python_hex, overlap='error')

    base_hex.write_hex_file(args.output)
