#!/usr/bin/env python3

'''
Combine the MicroPython firmware with a Python script and produce a hex file
ready for uploading to the micro:bit.

Usage: ./makecombinedhex.py <firmware.hex> <script.py> [-o <combined.hex>]

Output goes to stdout if no filename is given.
'''

import sys
import argparse
import hexlifyscript

def get_largest_addr(hexfile):
    largest_addr = 0
    for line in hexfile:
        count = int(line[1:3], 16)
        addr = int(line[3:7], 16)
        type = int(line[7:9], 16)
        if count == 2 and type == 4: # ext linear addr
            page = int(line[9:13], 16) << 16
        elif type == 0: # data
            # only count pages in flash, not in the UICR
            if page < 0x10000000:
                largest_addr = max(largest_addr, page + addr + count)
    return largest_addr

def find_uicr_line(hexfile):
    for i, line in enumerate(hexfile):
        # UICR from 0x10001000 so we expect an extended linear address record
        if ':020000041000EA' in line:
            return i
    return None

if __name__ == '__main__':
    arg_parser = argparse.ArgumentParser(description='Produce combined hex firmware for the micro:bit.')
    arg_parser.add_argument('-o', '--output', default=sys.stdout, type=argparse.FileType('wt'), help='output file (default is stdout)')
    arg_parser.add_argument('firmware', nargs=1, help='input MicroPython firmware')
    arg_parser.add_argument('script', nargs=1, help='input Python script')
    args = arg_parser.parse_args()

    # read in the firmware
    with open(args.firmware[0], 'rt') as f:
        firmware = f.readlines()

    # check the firmware is not too large
    if get_largest_addr(firmware) > hexlifyscript.SCRIPT_ADDR:
        raise Exception('firmware overflows into script region')

    # hexlify the script
    with open(args.script[0], 'rb') as f:
        script = hexlifyscript.hexlify_script(f.read())

    # print lines until UICR area or the start linear address record
    firmware_end = find_uicr_line(firmware) or len(firmware) - 2
    for line in firmware[:firmware_end]:
        print(line, end='', file=args.output)

    # print script
    print(script, file=args.output)

    # print rest of hex file
    for line in firmware[firmware_end:]:
        print(line, end='', file=args.output)
