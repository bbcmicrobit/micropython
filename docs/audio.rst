Audio
*******

.. py:module:: audio

This module allows you play sounds with the micro:bit.

By default sound output will be via the edge connector on pin 0 and the
built-in speaker **V2**. You can connect a wired headphones or a speaker to
pin 0 and GND on the edge connector to hear the sounds.

The ``audio`` module can be imported as ``import audio`` or accessed via
the ``microbit`` module as ``microbit.audio``.

Functions
=========

.. py:function:: play(source, wait=True, pin=pin0, return_pin=None)
                 play(source, wait=True, pin=(pin_speaker, pin0), return_pin=None)

    Play the source to completion.

    :param source: ``Sound``: The ``microbit`` module contains a list of
      built-in sounds that your can pass to ``audio.play()``.

      ``AudioFrame``: The source agrument can also be an iterable
      of ``AudioFrame`` elements as described below.
    :param wait: If ``wait`` is ``True``, this function will block until the
        source is exhausted.
    :param pin: As with the music module, you can use the optional ``pin``
      argument to specify the output pin can be used to override the
      default of ``microbit.pin0``. If you have the latest micro:bit **V2**,
      you can use ``microbit.pin_speaker``. The pin argument can also take a
      tuple of two pins, for example ``pin=(pin_speaker, pin0)`` which would
      output sound on the built-in speaker and pin 0.
    :param return_pin: specifies a differential edge connector pin to connect
      to an external speaker instead of ground. This is ignored for the **V2**
      revision.

.. py:function:: is_playing()

    :returns: ``True`` if audio is playing, otherwise returns ``False``.

.. py:function:: stop()

    Stops all audio playback.

Classes
=======

.. py:class::
    AudioFrame

    An ``AudioFrame`` object is a list of 32 samples each of which is an unsigned byte
    (whole number between 0 and 255).

    It takes just over 4 ms to play a single frame.

    .. py:function:: copyfrom(other)

        Overwrite the data in this ``AudioFrame`` with the data from another
        ``AudioFrame`` instance.

        :param other: ``AudioFrame`` instance from which to copy the data.


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

The ``audio`` module can consumes an iterable (sequence, like list or tuple, or
generator) of ``AudioFrame`` instances, each 32 samples at 7812.5 Hz, and uses
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
