Audio
*******

.. py:module:: audio

This module allows you play sounds from a speaker attached to the Microbit.
In order to use the audio module you will need to provide a sound source.

A sound source is an iterable (sequence, like list or tuple, or a generator) of
frames, each of 32 samples.
The ``audio`` modules plays samples at the rate of 7812.5 samples per second,
which means that it can reproduce frequencies up to 3.9kHz.

Functions
=========

.. py:function:: play(source, wait=True, pins=(pin0, pin1))

    Play the source to completion.

    ``source`` is an iterable, each element of which must be an ``AudioFrame``.

    If ``wait`` is ``True``, this function will block until the source is exhausted.

    ``pins`` specifies which pins the speaker is connected to.

Classes
=======

.. py:class::
    AudioFrame

    An ``AudioFrame`` object is a list of 32 samples each of which is a signed byte
    (whole number between -128 and 127).

    It takes just over 4 ms to play a single frame.

Using audio
===========

You will need a sound source, as input to the ``play`` function. You can generate your own, like in
``examples/waveforms.py`` or you can use the sound sources provided by modules like ``synth``.


Technical Details
=================

.. note::
    You don't need to understand this section to use the ``audio`` module.
    It is just here in case you wanted to know how it works.

The ``audio`` module consumes samples at 7812.5 kHz, and uses linear interpolation to
output a PWM signal at 32.5 kHz, which gives tolerable sound quality.

The function ``play`` fully copies all data from each ``AudioFrame`` before it
calls ``next()`` for the next frame, so a sound source can use the same ``AudioFrame``
repeatedly.

The ``audio`` module has an internal 64 sample buffer from which it reads samples.
When reading reaches the start or the mid-point of the buffer, it triggers a callback to
fetch the next ``AudioFrame`` which is then copied into the buffer.
This means that a sound source has under 4ms to compute the next ``AudioFrame``,
and for reliable operation needs to take less 2ms (which is 32000 cycles, so should be plenty).


Example
=======

.. include:: ../examples/waveforms.py
    :code: python

