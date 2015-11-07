Compass
*******

.. py:module:: microbit.compass

This module lets you access the build-in electronic compass. Before using,
the compass should be calibrated, otherwise the readings may be wrong.


Functions
=========

.. py:function:: calibrate()

    Starts the calibration process. After this is called, the user should
    lay the board flat on the table and rotate it horizontally, until the
    process is finished.

.. py:function:: is_calibrated()

    Returns ``True`` if the compass has been successfully calibrated, and
    returns ``False`` otherwise.


.. py:function:: is_calibrating()

    Returns ``True`` if the calibration process is in progress, and ``False``
    if it has finished or hasn't been started yet.


.. py:function:: clear_calibrarion()

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


Example
=======

.. include:: ../examples/compass.py
    :code: python
