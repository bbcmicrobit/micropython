Compass
*******

.. py:module:: microbit.compass

This module lets you access the built-in electronic compass. Before using,
the compass should be calibrated, otherwise the readings may be wrong.

.. warning::

    Calibrating the compass will cause your program to pause until calibration
    is complete. Calibration consists of a little game to draw a circle on the
    LED display by rotating the device.


Functions
=========

.. py:function:: calibrate()

    Starts the calibration process. An instructive message will be scrolled
    to the user after which they will need to rotate the device in order to
    draw a circle on the LED display.

.. py:function:: is_calibrated()

    Returns ``True`` if the compass has been successfully calibrated, and
    returns ``False`` otherwise.


.. py:function:: clear_calibration()

    Undoes the calibration, making the compass uncalibrated again.


.. py:function:: get_x()

    Gives the reading of the magnetic force on the ``x`` axis, as a
    positive or negative integer, depending on the direction of the
    force.


.. py:function:: get_y()

    Gives the reading of the magnetic force on the ``x`` axis, as a
    positive or negative integer, depending on the direction of the
    force.


.. py:function:: get_z()

    Gives the reading of the magnetic force on the ``x`` axis, as a
    positive or negative integer, depending on the direction of the
    force.


.. py:function:: heading()

    Gives the compass heading, calculated from the above readings, as an
    integer in the range from 0 to 360, representing the angle in degrees,
    clockwise, with north as 0.


.. py:function:: get_field_strength()

    Returns an integer indication of the magnitude of the magnetic field around
    the device.


Example
=======

.. include:: ../examples/compass.py
    :code: python
