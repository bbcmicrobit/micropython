Audio
*******

.. py:module:: audio

This module allows you to play your own sounds. If you are using a micro:bit
**V2**, ``audio`` is also part of the ``microbit`` module.

By default sound output will be via the edge connector on pin 0 and the
built-in speaker **V2**. You can connect a wired headphones or a speaker to
pin 0 and GND on the edge connector to hear the sounds.

Functions
=========

.. py:function:: play(source, wait=True, pin=pin0, return_pin=None)

    Play the source to completion.

    * **source**: ``Sound`` - The ``microbit`` module contains a list of
      built-in sounds that your can pass to ``audio.play()``.

    * **source**: ``AudioFrame`` - The source agrument can also be an iterable
      of ``AudioFrame`` elements as described below.

    * **wait**: If ``wait`` is ``True``, this function will block until the
      source is exhausted.

    * **pin**: As with the music module, you can use the optional ``pin``
      argument to specify the output pin can be used to override the
      default of ``microbit.pin0``.

    * **return_pin**: specifies a differential edge connector pin to connect
      to an external speaker instead of ground. This is ignored for the **V2**
      revision.

.. py:function:: is_playing()

    Return ``True`` if audio is playing, otherwise return ``False``.

.. py:function:: stop()
    
    Stops all audio playback.

Classes
=======

.. py:class::
    AudioFrame

    An ``AudioFrame`` object is a list of 32 samples each of which is a signed byte
    (whole number between -128 and 127).

    It takes just over 4 ms to play a single frame.

Using audio
===========

You will need a sound source, as input to the ``play`` function. You can use
the built-in sounds **V2** from the ``microbit`` module, ``microbit.Sound``, or
generate your own, like in ``examples/waveforms.py``.

Built-in sounds **V2**
----------------------

The built-in sounds can be called using ``audio.play(Sound.NAME)``.

* ``Sound.GIGGLE``
* ``Sound.HAPPY``
* ``Sound.HELLO``
* ``Sound.MYSTERIOUS``
* ``Sound.SAD``
* ``Sound.SLIDE``
* ``Sound.SOARING``
* ``Sound.SPRING``
* ``Sound.TWINKLE``
* ``Sound.YAWN``

Technical Details
=================

.. note::
    You don't need to understand this section to use the ``audio`` module.
    It is just here in case you wanted to know how it works.

The ``audio`` module consumes ``AudioFrame`` samples at 7812.5 Hz, and uses
linear interpolation to output a PWM signal at 32.5 kHz, which gives tolerable
sound quality.

The function ``play`` fully copies all data from each ``AudioFrame`` before it
calls ``next()`` for the next frame, so a sound source can use the same
``AudioFrame`` repeatedly.

The ``audio`` module has an internal 64 sample buffer from which it reads
samples. When reading reaches the start or the mid-point of the buffer, it
triggers a callback to fetch the next ``AudioFrame`` which is then copied into
the buffer. This means that a sound source has under 4ms to compute the next
``AudioFrame``, and for reliable operation needs to take less 2ms (which is
32000 cycles, so should be plenty).


Example
=======

.. include:: ../examples/waveforms.py
    :code: python
