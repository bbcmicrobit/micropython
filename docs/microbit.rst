Microbit Module
***************

.. py:module:: microbit


The ``microbit`` module gives you access to all the hardware that is built-in
into your board.


Functions
=========

.. py:function:: panic()

    Enter a panic mode. Requires restart.


.. py:function:: reset()

    Restart the board.


.. py:function:: sleep(n)

    Wait for ``n`` milliseconds. One second is 1000 milliseconds, so::

        microbit.sleep(1000)

    will pause the execution for one second.


.. py:function:: random(n)

    Generate a pseudo-random number between 0 and ``n``-1. For instance::

        result = microbit.random(6) + 1

    will simulate a roll of a six-sided die, giving numbers from 1 to 6
    inclusive.


.. py:function:: running_time()

    Return the number of milliseconds since the board was switched on or
    restarted.


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

    display.rst
    music.rst
    uart.rst
    i2c.rst
    accelerometer.rst
    compass.rst
