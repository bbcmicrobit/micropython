'''
Turn a Python script into Intel HEX format to be concatenated at the
end of the MicroPython firmware.hex.  A simple header is added to the
script.
'''

import sys
import struct
import binascii

# read script body
with open(sys.argv[1], "rb") as f:
    data = f.read()

# add header, pad to multiple of 16 bytes
data = b'MP' + struct.pack('<H', len(data)) + data
data = data + bytes(16 - len(data) % 16)
assert len(data) <= 0x2000

# convert to .hex format
addr = 0x3e000 # magic start address in flash
for i in range(0, len(data), 16):
    chunk = data[i:min(i + 16, len(data))]
    chunk = struct.pack('>BHB', len(chunk), addr & 0xffff, 0) + chunk
    checksum = (-(sum(data))) & 0xff
    hexline = ':%s%02X' % (str(binascii.hexlify(chunk), 'utf8').upper(), checksum)
    print(hexline)
    addr += 16
