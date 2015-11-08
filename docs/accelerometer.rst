Accelerometer
*************

.. py:module:: microbit.accelerometer

This module gives you access to the on-board accelerometer.


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
    tuple of integers.

Example
-------

.. include:: ../examples/simple_slalom.py
    :code: python
