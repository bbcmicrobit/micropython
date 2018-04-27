/*
  Portions of this code based on code provided under MIT license from Microsoft
  https://github.com/Microsoft/pxt-ws2812b

  MIT License

  Copyright (c) Microsoft Corporation. All rights reserved.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.'

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE
*/

.global sendNeopixelBuffer

 /* declared as extern void sendBuffer(uint32_t pin, uint8_t* data_address, uint16_t num_leds) */

sendNeopixelBuffer:
    push {r0, r1, r2, r3, r4, r5, r6}

    /*
    We are expecting from the callee:
    r0 = pinmask to waggle in the GPIO register
    r1 = address of the data we are supposed to be sending
    r2 = number of LEDs

    Setup the initial values
     r1 = Pin mask in the GPIO register
     r2 = GPIO clear register
     r3 = GPIO SET
     r4 = Address pointer for the data - we cast this as a byte earlier because it really is.
     r5 = Length of the data (number of LEDS * 3 bytes per LED) - If trying to add RGB+W this sum might need to be done conditionally
     r6 = Parallel to serial conversion mask
    */
    mov r4, r1
    mov r6, #3
    mul r6, r2, r6
    mov r5, r6

    /*load the pin set and clr addresses by a cunning combo of shifts and adds*/
    movs r3, #160
    movs r1, #0x0c
    lsl r3, r3, #15
    add r3, #05
    lsl r3, r3, #8
    add r2, r3, r1
    add r3, #0x08

    mov r1, r0 /* finally move the pin mask from r0 to r1*/

    /*
    This code serialises the data bits for each LED.
    The data byte is loaded in the common section (label .common) and then each bit is masked and tested for '0' (label .nextbit)
    If it is a '0' we turn off the pin asap and then move to the code that advances to the next bit/byte. If a '1' we leave the pin on and do the same thing.
    If the mask (r6) is still valid then we are still moving out the current byte, so repeat.
    If it is '0' then we have done this byte and need to load the next byte from the pointer in r4.
    r5 contains the count of bytes - calculated above from num LEDs * 3 bytes per LED.
    --If this code needs to do RGB+W LEDS then that will need to be addressed.
    Once we run r5 down to '0' we exit the data shifting and return.
    */

    mrs r6, PRIMASK /* disable interrupts whilst we mess with timing critical waggling. */
    push {r6}
    cpsid i

    b .start

    .nextbit:
        str r1, [r3, #0]
        tst r6, r0
        bne .bitisone
        str r1, [r2, #0]
    .bitisone:
        lsr r6, #1
        bne .justbit

        add r4, #1
        sub r5, #1
        beq .stop

    .start:
        movs r6, #0x80
        nop

    .common:
        str r1, [r2, #0]
        ldrb r0, [r4, #0]
        b .nextbit

    .justbit:
        b .common

    .stop:
        str r1, [r2, #0]

    pop {r6}
    msr PRIMASK, r6
    pop {r0, r1, r2, r3, r4, r5, r6}
