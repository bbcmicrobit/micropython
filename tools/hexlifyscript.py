#!/usr/bin/env python3

'''
Turn a Python script into Intel HEX format to be concatenated at the
end of the MicroPython firmware.hex.  A simple header is added to the
script.

To execute from command line: ./hexlifyscript.py <script.py>
It also accepts data on standard input.
'''

import struct
import binascii
import fileinput


SCRIPT_ADDR = 0x3e000 # magic start address in flash of script

def hexlify_script(script):
    # add header, pad to multiple of 16 bytes
    data = b'MP' + struct.pack('<H', len(script)) + script
    data = data + bytes(16 - len(data) % 16)
    assert len(data) <= 0x2000

    # convert to .hex format
    output = []
    addr = SCRIPT_ADDR
    assert(SCRIPT_ADDR >> 16 == 3) # 0x0003 is hard coded in line below
    output.append(':020000040003F7') # extended linear address, 0x0003
    for i in range(0, len(data), 16):
        chunk = data[i:min(i + 16, len(data))]
        chunk = struct.pack('>BHB', len(chunk), addr & 0xffff, 0) + chunk
        checksum = (-(sum(chunk))) & 0xff
        hexline = ':%s%02X' % (str(binascii.hexlify(chunk), 'utf8').upper(), checksum)
        output.append(hexline)
        addr += 16

    return '\n'.join(output)

if __name__ == '__main__':
    # read script from a file and print out the hexlified version
    with fileinput.input(mode='rb') as lines:
        print(hexlify_script(b''.join(lines)))
