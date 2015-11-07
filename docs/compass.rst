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

    Gives the reading on the ``x`` axis of the compass.


.. py:function:: get_y()

    Gives the reading on the ``y`` axis of the compass.


.. py:function:: get_z()

    Gives the reading on the ``z`` axis of the compass (vertical).


.. py:function:: heading()

    Gives the compass heading, calculated from the above readings.


Example
=======

.. include:: ../examples/compass.py
    :code: python
