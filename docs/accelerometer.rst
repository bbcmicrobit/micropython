Accelerometer
*************

.. py:module:: microbit.accelerometer

This module gives you access to the on-board accelerometer. As a convenience
the accelerometer provides convenience functions for detecting gestures. The
recognised gestures are: ``up``, ``down``, ``left``, ``right``, ``face up``,
``face down``, ``freefall``, ``3g``, ``6g``, ``8g``, ``shake``.


Functions
=========

.. py:function:: get_x()

    Get the acceleration measurement in the ``x`` axis, as a positive or
    negative integer, depending on the direction.


.. py:function:: get_y()

    Get the acceleration measurement in the ``y`` axis, as a positive or
    negative integer, depending on the direction.


.. py:function:: get_z()

    Get the acceleration measurement in the ``z`` axis, as a positive or
    negative integer, depending on the direction.

.. py:function:: get_values()

    Get the acceleration measurements in all axes at once, as a three-element
    tuple of integers ordered as X, Y, Z.

.. py:function:: current_gesture()

    Return the name of the current gesture.

.. py:function:: is_gesture()

    Return True or False to indicate if the named gesture is currently active.

.. py:function:: was_gesture(name)

    Return True or False to indicate if the named gesture was active since the
    last call.

.. py:function:: get_gestures()

    Return a tuple of the gesture history. The most recent is listed last.

.. py:function:: reset_gestures()

    Clears the gesture history.

Example
-------

.. include:: ../examples/simple_slalom.py
    :code: python
