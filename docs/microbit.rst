Microbit Module
***************

.. py:module:: microbit


The ``microbit`` module gives you access to all the hardware that is built-in
into your board.


Functions
=========

.. py:function:: panic(n)

    Enter a panic mode. Requires restart. Pass in an arbitrary integer <= 255
    to indicate a status::

        microbit.panic(255)


.. py:function:: reset()

    Restart the board.


.. py:function:: running_time()

    Return the number of milliseconds since the board was switched on or
    restarted.


.. py:function:: sleep(n)

    Wait for ``n`` milliseconds. One second is 1000 milliseconds, so::

        microbit.sleep(1000)

    will pause the execution for one second.  ``n`` can be an integer or
    a floating point number.


.. py:function:: temperature()

    Return the temperature of the micro:bit in degrees Celcius.


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
