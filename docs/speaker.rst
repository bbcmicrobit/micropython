Speaker **V2**
**************

.. py:module:: speaker

The micro:bit **V2** has a built-in speaker located on the rear of the board.

.. image:: speaker.png
    :width: 300px
    :align: center
    :height: 240px
    :alt: speaker on rear of micro:bit V2

By default sound output will be via the edge connector on pin 0 and the
built-in speaker **V2**. You can connect wired headphones or a speaker to
pin 0 and GND on the edge connector to hear the sounds.

The speaker can be turned off or on using the ``speaker`` object.

Functions
=========

.. py:function:: off()

    Use off() to turn off the speaker. The

.. py:function:: on()

    Use on() to turn off the display (thus allowing you to re-use the GPIO
    pins associated with the display for other purposes).

.. py:function:: is_on()

    Returns ``True`` if the speaker is on, otherwise returns ``False``.