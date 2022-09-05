Accelerometer
*************

.. py:module:: microbit.accelerometer

This object gives you access to the on-board accelerometer.

By default MicroPython sets the accelerometer range to +/- 2000 mg (``g`` being
a unit of acceleration based on the standard gravity), which configures the
maximum and minimum values returned by the accelerometer functions.
The range can be changed via :func:`microbit.accelerometer.set_range`.

The accelerometer also provides convenience functions for detecting gestures.
The recognised gestures are represented as strings:
``up``, ``down``, ``left``, ``right``, ``face up``, ``face down``,
``freefall``, ``3g``, ``6g``, ``8g``, ``shake``.

.. note::

    Gestures are not updated in the background so there needs to be constant
    calls to some accelerometer method to do the gesture detection.
    Usually gestures can be detected using a loop with a small
    :func:`microbit.sleep` delay.

Functions
=========

.. py:function:: get_x()

    :returns: The acceleration measurement in the ``x`` axis in milli-g,
        as a positive or negative integer.

.. py:function:: get_y()

    :returns: The acceleration measurement in the ``y`` axis in milli-g,
        as a positive or negative integer.

.. py:function:: get_z()

    :returns: The acceleration measurement in the ``z`` axis in milli-g,
        as a positive or negative integer.

.. py:function:: get_values()

    :returns: The acceleration measurements in all axes at once, as a
        three-element tuple of integers ordered as X, Y, Z.

.. py:function:: get_strength()

    Get the acceleration measurement of all axes combined, as a positive
    integer.  This is the Pythagorean sum of the X, Y and Z axes.

    :returns: The combined acceleration strength of all the axes, in milli-g.

.. py:function:: current_gesture()

    :returns: String with the name of the current gesture.

.. py:function:: is_gesture(name)

    :param name: String with the name of the gesture to check.
    :return: Boolean indicating if the named gesture is currently active.

.. py:function:: was_gesture(name)

    :param name: String with the name of the gesture to check.
    :return: Boolean indicating if the named gesture was active since the
        last call.

.. py:function:: get_gestures()

    Get a historical list of the registered gestures.

    Calling this function clears the gesture history before returning.

    :returns: A tuple of the gesture history, most recent is listed last.

.. py:function:: set_range(value)

    Set the accelerometer sensitivity range, in g (standard gravity), to the
    closest values supported by the hardware, so it rounds to either
    ``2``, ``4``, or ``8`` g.

    :param value: New range for the accelerometer, an integer in ``g``.

Examples
========

A fortune telling magic 8-ball. Ask a question then shake the device for an
answer.

.. include:: ../examples/magic8.py
    :code: python

Simple Slalom. Move the device to avoid the obstacles.

.. include:: ../examples/simple_slalom.py
    :code: python
