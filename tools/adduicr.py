#!/usr/bin/env python3

'''
Add a UICR region to the hex firmware for MicroPython on the micro:bit.

Usage: ./adduicr.py <firmware.hex> <version_string_address> [-o <combined.hex>]

Output goes to stdout if no filename is given.
'''

import sys
import struct
import argparse
import hexlifyscript
import makecombinedhex

NRF_PAGE_SIZE_LOG2 = 10
NRF_PAGE_SIZE = 1 << NRF_PAGE_SIZE_LOG2
UICR_BASE_ADDR = 0x100010c0
UICR_MAGIC_NUMBER = 0x17eeb07c

if __name__ == '__main__':
    arg_parser = argparse.ArgumentParser(description='Add UICR region to hex firmware for the micro:bit.')
    arg_parser.add_argument('-o', '--output', default=sys.stdout, type=argparse.FileType('wt'), help='output file (default is stdout)')
    arg_parser.add_argument('firmware', nargs=1, help='input MicroPython firmware')
    arg_parser.add_argument('address', nargs=1, type=lambda x: int(x, 0), help='address in flash of the version string')
    args = arg_parser.parse_args()

    # read in the firmware
    with open(args.firmware[0], 'rt') as f:
        firmware = f.readlines()

    # print head of firmware
    for line in firmware[:-2]:
        print(line, end='', file=args.output)

    # make UICR data
    uicr_data = b''
    uicr_data += struct.pack('<III', UICR_MAGIC_NUMBER, 0xffffffff, NRF_PAGE_SIZE_LOG2)
    uicr_data += struct.pack('<HHII', 0, (makecombinedhex.get_largest_addr(firmware) + NRF_PAGE_SIZE - 1) // NRF_PAGE_SIZE, 0xffffffff, args.address[0])
    uicr_data += struct.pack('<I', 0)

    # print UICR data
    print(hexlifyscript.make_ihex_record(0, hexlifyscript.IHEX_TYPE_EXT_LIN_ADDR, struct.pack('>H', UICR_BASE_ADDR >> 16)), file=args.output)
    for i in range(0, len(uicr_data), 16):
        chunk = uicr_data[i:min(i + 16, len(uicr_data))]
        print(hexlifyscript.make_ihex_record(UICR_BASE_ADDR + i, hexlifyscript.IHEX_TYPE_DATA, chunk), file=args.output)

    # print tail of firmware
    print(firmware[-2], end='', file=args.output)
    print(firmware[-1], end='', file=args.output)
