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

    Sleep for the given number of seconds. Some boards may accept seconds as a 
    floating-point number to sleep for a fractional number of seconds. Note that 
    other boards may not accept a floating-point argument, for compatibility 
    with them use :func:`utime.sleep_ms()` and :func:`utime.sleep_us()`
    functions.


.. method:: utime.sleep_ms(ms)

    Delay for given number of milliseconds, should be positive or 0.


.. method:: utime.sleep_us(us)

    Delay for given number of microseconds, should be positive or 0.


.. method:: utime.ticks_ms()

    Returns an increasing millisecond counter with an arbitrary reference point, 
    that wraps around after some value.

    The wrap-around value is not explicitly exposed, but we will refer to it as 
    *TICKS_MAX* to simplify discussion. Period of the values is 
    *TICKS_PERIOD = TICKS_MAX + 1*. *TICKS_PERIOD* is guaranteed to be a power 
    of two, but otherwise may differ from port to port. The same period value 
    is used for :func:`utime.ticks_ms()` and :func:`utime.ticks_us()` for 
    simplicity. Thus, these functions will return a value in range 
    *[0 .. TICKS_MAX]*, inclusive, total *TICKS_PERIOD* values. Note that only 
    non-negative values are used. For the most part, you should treat values 
    returned by these functions as opaque. The only operations available for 
    them are :func:`utime.ticks_diff()` and :func:`utime.ticks_add()` functions
    described below.


.. method:: utime.ticks_us()

    Just like :func:`utime.ticks_ms()` above, but in microseconds.


.. method:: utime.ticks_add(ticks, delta)

    Offset ticks value by a given number, which can be either positive or 
    negative. Given a ticks value, this function allows to calculate ticks 
    value delta ticks before or after it, following modular-arithmetic 
    definition of tick values (see :func:`utime.ticks_ms()` above). ticks 
    parameter must be a direct result of call to :func:`utime.ticks_ms()` or 
    :func:`utime.ticks_us()` functions (or from previous call to 
    :func:`utime.ticks_add()`). However, delta can be an arbitrary integer 
    number or numeric expression. :func:`utime.ticks_add()` is useful for 
    calculating deadlines for events/tasks. (Note: you must use 
    :func:`utime.ticks_diff()` function to work with deadlines.)

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
    However, values returned by ``ticks_ms()``, etc. functions may wrap around,
    so directly using subtraction on them will produce incorrect result. That 
    is why ``ticks_diff()`` is needed, it implements modular (or more 
    specifically, ring) arithmetics to produce correct result even for 
    wrap-around values (as long as they not too distant inbetween, see below).
    The function returns **signed** value in the range 
    *[-TICKS_PERIOD/2 .. TICKS_PERIOD/2-1]* (that’s a typical range definition 
    for two’s-complement signed binary integers). If the result is negative, it
    means that *ticks1* occurred earlier in time than *ticks2*. Otherwise, it 
    means that *ticks1* occurred after *ticks2*. This holds **only** if 
    *ticks1* and *ticks2* are apart from each other for no more than 
    *TICKS_PERIOD/2-1* ticks. If that does not hold, incorrect result will be
    returned. Specifically, if two tick values are apart for *TICKS_PERIOD/2-1*
    ticks, that value will be returned by the function. However, if
    *TICKS_PERIOD/2* of real-time ticks has passed between them, the function 
    will return *-TICKS_PERIOD/2* instead, i.e. result value will wrap around 
    to the negative range of possible values.

    Informal rationale of the constraints above: Suppose you are locked in a 
    room with no means to monitor passing of time except a standard 12-notch 
    clock. Then if you look at dial-plate now, and don’t look again for another
    13 hours (e.g., if you fall for a long sleep), then once you finally look 
    again, it may seem to you that only 1 hour has passed. To avoid this 
    mistake, just look at the clock regularly. Your application should do the 
    same. “Too long sleep” metaphor also maps directly to application behavior:
    don’t let your application run any single task for too long. Run tasks in 
    steps, and do time-keeping inbetween.

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