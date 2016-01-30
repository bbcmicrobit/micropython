Display
*******

.. py:module:: microbit.display

This module controls the 5×5 LED display on the front of your board. It can
be used to display images, animations and even text.

.. image:: scroll-hello.gif

Functions
=========

.. py:function:: get_pixel(x, y)

    Return the brightness of the LED at column ``x`` and row ``y`` as an
    integer between 0 (off) and 9 (bright).


.. py:function:: set_pixel(x, y, value)

    Set the brightness of the LED at column ``x`` and row ``y`` to ``value``,
    which has to be an integer between 0 and 9.


.. py:function:: clear()

    Set the brightness of all LEDs to 0 (off).

.. py:function:: show(image)

    Display the ``image``.


.. py:function:: show(iterable, delay, wait=True, loop=False, clear=False)

    Display images or letters from the ``iterable`` in sequence, with ``delay``
    milliseconds between them.

    If ``wait`` is ``True``, this function will block until the animation is
    finished, otherwise the animation will happen in the background.

    If ``loop`` is ``True``, the animation will repeat forever.

    If ``clear`` is ``True``, the display will be cleared after the iterable has finished.


.. py:function:: scroll(string, delay=400)

    Similar to ``show``, but scrolls the ``string`` horizontally instead. The
    ``delay`` parameter controls how fast the text is scrolling. This function
    blocks until it is finished.
