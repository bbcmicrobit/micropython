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


.. py:function:: sleep(n)

    Wait for ``n`` milliseconds. One second is 1000 milliseconds, so
    ``microbit.sleep(1000)`` will pause the execution for one second.

    :param n: An integer or floating point number indicating the number of
        milliseconds to wait.


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


Modules
=======

.. toctree::
    :maxdepth: 1

    accelerometer.rst
    compass.rst
    display.rst
    i2c.rst
    spi.rst
    uart.rst
