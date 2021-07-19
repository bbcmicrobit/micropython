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


.. py:function:: sleep(n)

    Wait for ``n`` milliseconds. One second is 1000 milliseconds, so::

        microbit.sleep(1000)

    will pause the execution for one second.  ``n`` can be an integer or
    a floating point number.


.. py:function:: running_time()

    Return the number of milliseconds since the board was switched on or
    restarted.

.. py:function:: run_every(n)

    Scheduler to run a piece of code at a set interval.

    This function can be used in two ways:
    * As a decorator, without the positional argument. For example,
      ``run_every(h=1, min=20, s=30, ms=50)``

    * As a function passing the callback as a positional argument.
      For example, run_every(foo, h=1, min=20, s=30, ms=50)

    This will schedule a function to be called at a given interval.
    Arguments with different time units are additive.

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
    log.rst
    microphone.rst
    speaker.rst
    spi.rst
    uart.rst
