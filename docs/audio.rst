Audio
*******

.. py:module:: audio

This module allows you play sounds from a speaker attached to the micro:bit.

The ``audio`` module can be imported as ``import audio`` or accessed via
the ``microbit`` module as ``microbit.audio``.

In order to use the audio module you will need to provide a sound source.

A sound source is an iterable (sequence, like list or tuple, or a generator) of
frames, each of 32 samples.
The ``audio`` modules plays samples at the rate of 7812.5 samples per second,
which means that it can reproduce frequencies up to 3.9kHz.

Functions
=========

.. py:function:: play(source, wait=True, pin=pin0, return_pin=None)

    Play the source to completion.

    :param source: An iterable sound source, each element of which must be
        an ``AudioFrame``.
    :param wait: If ``wait`` is ``True``, this function will block until the
        source is exhausted.
    :param pin: Specifies which pin the speaker is connected to.
    :param return_pin: Specifies a differential pin to connect to the speaker
        instead of ground.

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

You will need a sound source, as input to the ``play`` function. You can generate your own, like in
``examples/waveforms.py``.


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
