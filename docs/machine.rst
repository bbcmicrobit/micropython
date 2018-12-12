..
   MicroPython license information
   ===============================

   The MIT License (MIT)

   Copyright (c) 2013-2017 Damien P. George, and others

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
   
   
Machine
*******

.. py:module:: machine

The machine module contains specific functions related to the micro:bit 
hardware. Most functions in this module allow to achieve direct and 
unrestricted access to and control of hardware blocks on a system (like CPU, 
timers, buses, etc.). Used incorrectly, this can lead to malfunction, lockups, 
crashes of your board, and in extreme cases, hardware damage.


Functions
=========

.. method:: machine.unique_id()

    Returns a byte string with a unique identifier of a board. It will vary 
    from one board instance to another.


.. method:: machine.reset()

    Resets the device in a manner similar to pushing the external RESET button.


.. method:: machine.freq()

    Returns CPU frequency in hertz.


.. method:: machine.disable_irq()

    Disable interrupt requests. Returns the previous IRQ state which should be 
    considered an opaque value. This return value should be passed to the 
    :func:`machine.enable_irq()` function to restore interrupts to their 
    original state, before :func:`machine.disable_irq()` was called.


.. method:: machine.enable_irq()

    Re-enable interrupt requests. The *state* parameter should be the value 
    that was returned from the most recent call to the
    :func:`machine.disable_irq()` function.


.. method:: machine.time_pulse_us(pin, pulse_level, timeout_us=1000000)

    Time a pulse on the given *pin*, and return the duration of the pulse in 
    microseconds. The *pulse_level* argument should be 0 to time a low pulse or
    1 to time a high pulse.

    If the current input value of the pin is different to *pulse_level*, the 
    function first (*) waits until the pin input becomes equal to 
    *pulse_level*, then (**) times the duration that the pin is equal to 
    *pulse_level*. If the pin is already equal to *pulse_level* then timing 
    starts straight away.

    The function will return -2 if there was timeout waiting for condition 
    marked (*) above, and -1 if there was timeout during the main measurement, 
    marked (**) above. The timeout is the same for both cases and given by 
    *timeout_us* (which is in microseconds).


Reading Memory
==============

The ``machine`` module allows you to read from the device's memory, getting 1 
byte (8 bits; ``mem8``), 2 byte (16 bits; ``mem16``), or 4 byte (32 bits; 
``mem32``) words from physical addresses. For example: ``mem8[0x00]`` reads 1 
byte on physical address ``0x00``. This has a number of uses, for example if 
you'd like to read data from the nRF51 registers.