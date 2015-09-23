#!/usr/bin/env python3
'''
Produce a combined HEX file.
'''
import os
import sys

from intelhex import IntelHex

if __name__ == "__main__":
    if len(sys.argv) not in (3, 4):
        print("Usage: makecombinedhex.py hexfile pythonfile [addr (default:0x3e000)]")
        sys.exit(1)

    hex_file = sys.argv[1]
    py_file = sys.argv[2]

    if len(sys.argv) != 4:
        py_addr = 0x3e000
    else:
        py_addr = int(sys.argv[3], 16)


    base_hex = IntelHex(hex_file)
    python_hex = IntelHex()

    python_hex.puts(py_addr, 'MP')

    with open(py_file, 'rb') as f:
        data = bytes(f.read())
        data = b'MP' + len(data).to_bytes(2, 'little') + data + bytes(16 - len(data) % 16)
        python_hex.puts(py_addr, data)

    base_hex.merge(python_hex, overlap='error')

    #print(base_hex.dump())

    name, ext = os.path.splitext(hex_file)

    new_name = name + '.combined' + ext

    base_hex.write_hex_file(new_name)
