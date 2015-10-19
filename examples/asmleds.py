"""
This script uses the inline assembler to make the LEDs light up
in a pattern based on how they are multiplexed in rows/cols.
"""

# row pins: 13, 14, 15
# col pins: 4..12 inclusive
# GPIO words starting at 0x50000500:
#   RESERVED, OUT, OUTSET, OUTCLR, IN, DIR, DIRSET, DIRCLR

@micropython.asm_thumb
def led_cycle():
    b(START)

    # DELAY routine
    label(DELAY)
    mov(r3, 0xa0)
    lsl(r3, r3, 11)
    label(delay_loop)
    sub(r3, 1)
    bne(delay_loop)
    bx(lr)

    label(START)

    cpsid('i')          # disable interrupts so we control the display

    mov(r0, 0x50)       # r0=0x50
    lsl(r0, r0, 16)     # r0=0x500000
    add(r0, 0x05)       # r0=0x500005
    lsl(r0, r0, 8)      # r0=0x50000500 -- this points to GPIO registers
    mov(r1, 0b111)
    lsl(r1, r1, 13)     # r1=0xe000
    str(r1, [r0, 8])    # pull all rows high

    mov(r1, 1 << 4)     # r1 holds current col bit
    mov(r2, 9)          # r2 holds number of cols left
    label(loop_on)
    str(r1, [r0, 12])   # pull col low to turn LEDs on
    bl(DELAY)           # wait
    lsl(r1, r1, 1)      # shift to next col
    sub(r2, 1)          # decrease col counter
    bne(loop_on)        # loop while there are still cols left

    mov(r1, 1 << 4)     # r1 holds current col bit
    mov(r2, 9)          # r2 holds number of cols left
    label(loop_off)
    str(r1, [r0, 8])    # pull col high to turn LEDs off
    bl(DELAY)           # wait
    lsl(r1, r1, 1)      # shift to next col
    sub(r2, 1)          # decrease col counter
    bne(loop_off)       # loop while there are still cols left

    cpsie('i')      # enable interrupts

for i in range(4):
    led_cycle()
