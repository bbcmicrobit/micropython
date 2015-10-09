#!/usr/bin/env python

# checks to see whether the micropython binary is so big that 
# it interferes with the address associated with the appended
# binary file.

import sys

# sudo pip install pyelftools
from elftools.elf.elffile import ELFFile


if __name__ == "__main__":
    if len(sys.argv) not in (2, 3):
        print('Usage: outputcheck.py elffile [pyfileaddr (default: 0x3e000)]')
        sys.exit(1)

    if len(sys.argv) != 3:
        max_value = 0x3e000
    else:
        max_value = int(sys.argv[2], 16)

    with open(sys.argv[1], 'rb') as f:
        elffile = ELFFile(f)

        # check the start address

        start = elffile.header['e_entry']
        length = sum(section.header.sh_size for section in elffile.iter_sections() if section.header.sh_flags & 0x2)

        end_addr = start + length

        print('start address: 0x%x' % start)
        print('flash size: 0x%x' % length)
        print('end address: 0x%x' % end_addr)
        print('python file address: 0x%x' % max_value)

        if end_addr < max_value:
            print("pass")
        else:
            print("fail")
            sys.exit(2)
