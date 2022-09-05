Microbit Module
***************

.. py:module:: microbit


The ``microbit`` module gives you access to all the hardware that is built-in
into your board.


Functions
=========

.. py:function:: panic(n)

    Enter a panic mode that stops all execution, scrolls an error code in
    the micro:bit display and requires restart::

        microbit.panic(255)

    :param n: An arbitrary integer between 0 and 255 to indicate an error code.


.. py:function:: reset()

    Restart the board.


.. py:function:: running_time()

    :returns: The number of milliseconds since the board was switched on or
        restarted.

.. py:function:: scale(value, from_, to)

    Converts a value from a range to another range.

    For example, to convert 30 degrees from Celsius to Fahrenheit::

        temp_fahrenheit = scale(30, from_=(0.0, 100.0), to=(32.0, 212.0))

    This can be useful to convert values between inputs and outputs,
    for example an accelerometer x value to a speaker volume.

    If one of the numbers in the ``to`` parameter is a floating point
    (i.e a decimal number like ``10.0``), this function will return a
    floating point number.
    If they are both integers (i.e ``10``), it will return an integer::

        returns_int = scale(accelerometer.get_x(), from_=(-2000, 2000), to=(0, 255))

    Negative scaling is also supported, for example
    ``scale(25, from_=(0, 100), to=(0, -200))`` will return ``-50``.

    :param value: A number to convert.
    :param from_: A tuple to define the range to convert from.
    :param to: A tuple to define the range to convert to.

    :returns: The ``value`` converted to the ``to`` range.

.. py:function:: set_volume(volume)

    (**V2 only**) Configure the output volume of the micro:bit speaker and
    pins::

        microbit.set_volume(127)

    :param volume: An integer between 0 and 255 to set the volume.


.. py:function:: sleep(n)

    Wait for ``n`` milliseconds. One second is 1000 milliseconds, so
    ``microbit.sleep(1000)`` will pause the execution for one second.

    :param n: An integer or floating point number indicating the number of
        milliseconds to wait.

.. py:function:: run_every(callback, h=None, min=None, s=None, ms=None)

    Schedule to run a function at the interval specified by the time arguments.

    ``run_every`` can be used in two ways:

    * As a **Decorator** - placed on top of the function to schedule.
      For example::

        @run_every(h=1, min=20, s=30, ms=50)
        def my_function():
            # Do something here

    * As a **Function** - passing the callback as a positional argument.
      For example::

        def my_function():
            # Do something here
        run_every(my_function, s=30)

    Each arguments corresponds to a different time unit and they are additive.
    So ``run_every(min=1, s=30)`` schedules the callback every minute and
    a half.

    When an exception is thrown inside the callback function it deschedules
    the function. To avoid this you can catch exceptions with ``try/except``.

    :param callback: Function to call at the provided interval.
    :param h: Sets the hour mark for the scheduling.
    :param min: Sets the minute mark for the scheduling.
    :param s: Sets the second mark for the scheduling.
    :param ms: Sets the millisecond mark for the scheduling.

.. py:function:: temperature()

    :returns: An integer with the temperature of the micro:bit in degrees
        Celcius.


Attributes
==========

.. toctree::
    :maxdepth: 1

    button.rst
    pin.rst


Classes
=======

.. toctree::
    :maxdepth: 1

    image.rst
    Sound <audio.rst>


Modules
=======

.. toctree::
    :maxdepth: 1

    accelerometer.rst
    Audio V2 <audio.rst>
    compass.rst
    display.rst
    i2c.rst
    microphone.rst
    speaker.rst
    spi.rst
    uart.rst
