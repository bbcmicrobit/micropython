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


utime
*****

.. py:module:: utime

The ``utime`` module provides functions for getting the current time and date, 
measuring time intervals, and for delays.

.. note::
    The ``utime`` module is a MicroPython implementation of the standard Python 
    ``time`` module. It can be imported using both ``import utime`` and 
    ``import time``, but the module is the same.


Functions
=========

.. method:: utime.sleep(seconds)

    Sleep for the given number of seconds. You can use a floating-point number 
    to sleep for a fractional number of seconds, or use the 
    :func:`utime.sleep_ms()` and :func:`utime.sleep_us()` functions.


.. method:: utime.sleep_ms(ms)

    Delay for given number of milliseconds, should be positive or 0.


.. method:: utime.sleep_us(us)

    Delay for given number of microseconds, should be positive or 0.


.. method:: utime.ticks_ms()

    Returns an increasing millisecond counter with an arbitrary reference point, 
    that wraps around after some value.


.. method:: utime.ticks_us()

    Just like :func:`utime.ticks_ms()` above, but in microseconds.


.. method:: utime.ticks_add(ticks, delta)

    Offset ticks value by a given number, which can be either positive or 
    negative. Given a ticks value, this function allows to calculate ticks 
    value delta ticks before or after it, following modular-arithmetic 
    definition of tick values.

    Example:

    .. code-block:: python

        # Find out what ticks value there was 100ms ago
        print(ticks_add(time.ticks_ms(), -100))

        # Calculate deadline for operation and test for it
        deadline = ticks_add(time.ticks_ms(), 200)
        while ticks_diff(deadline, time.ticks_ms()) > 0:
            do_a_little_of_something()

        # Find out TICKS_MAX used by this port
        print(ticks_add(0, -1))


.. method:: utime.ticks_diff(ticks1, ticks2)

    Measure ticks difference between values returned from 
    :func:`utime.ticks_ms()` or :func:`ticks_us()` functions, as a signed value
    which may wrap around.

    The argument order is the same as for subtraction operator, 
    ``ticks_diff(ticks1, ticks2)`` has the same meaning as ``ticks1 - ticks2``.

    :func:`utime.ticks_diff()` is designed to accommodate various usage 
    patterns, among them:

    Polling with timeout. In this case, the order of events is known, and you
    will deal only with positive results of :func:`utime.ticks_diff()`:

    .. code-block:: python

        # Wait for GPIO pin to be asserted, but at most 500us
        start = time.ticks_us()
        while pin.value() == 0:
            if time.ticks_diff(time.ticks_us(), start) > 500:
                raise TimeoutError


    Scheduling events. In this case, :func:`utime.ticks_diff()` result may be
    negative if an event is overdue:


    .. code-block:: python

        # This code snippet is not optimized
        now = time.ticks_ms()
        scheduled_time = task.scheduled_time()
        if ticks_diff(scheduled_time, now) > 0:
            print("Too early, let's nap")
            sleep_ms(ticks_diff(scheduled_time, now))
            task.run()
        elif ticks_diff(scheduled_time, now) == 0:
            print("Right at time!")
            task.run()
        elif ticks_diff(scheduled_time, now) < 0:
            print("Oops, running late, tell task to run faster!")
            task.run(run_faster=true)