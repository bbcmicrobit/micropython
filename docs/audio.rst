Audio
*******

.. py:module:: audio

This module allows you play sounds with the micro:bit.

By default sound output will be via the edge connector on pin 0 and the
:doc:`built-in speaker <speaker>` (**V2**). You can connect wired headphones or
a speaker to pin 0 and GND on the edge connector to hear the sounds.

The ``audio`` module can be imported as ``import audio`` or accessed via
the ``microbit`` module as ``microbit.audio``.

There are three different kinds of audio sources that can be played using the
:py:meth:`audio.play` function:

1. `Built in sounds <#built-in-sounds-v2>`_ (**V2**),
   e.g. ``audio.play(Sound.HAPPY)``
2. `Sound Effects <#sound-effects-v2>`_ (**V2**), a way to create custom sounds
   by configuring its parameters::

    my_effect = audio.SoundEffect(freq_start=400, freq_end=2500, duration=500)
    audio.play(my_effect)

3. `Audio Frames <#audioframe>`_, an iterable (like a list or a generator)
   of Audio Frames, which are lists of 32 samples with values from 0 to 255::

    square_wave = audio.AudioFrame()
    for i in range(16):
        square_wave[i] = 0
        square_wave[i + 16] = 255
    audio.play([square_wave] * 64)


Functions
=========

.. py:function:: play(source, wait=True, pin=pin0, return_pin=None)

    Play the audio source to completion.

    :param source: There are three types of data that can be used as a source:

        - ``Sound``: The ``microbit`` module contains a list of
          built-in sounds, e.g. ``audio.play(Sound.TWINKLE)``. A full list can
          be found in the `Built in sounds <#built-in-sounds-v2>`_ section.
        - ``SoundEffect``: A sound effect, or an iterable of sound effects,
          created via the :py:meth:`audio.SoundEffect` class
        - ``AudioFrame``: An iterable of ``AudioFrame`` instances as described
          in the `AudioFrame Technical Details <#id2>`_ section

    :param wait: If ``wait`` is ``True``, this function will block until the
        source is exhausted.

    :param pin: An optional argument to specify the output pin can be used to
      override the default of ``pin0``. If we do not want any sound to play
      we can use ``pin=None``.

    :param return_pin: specifies a differential edge connector pin to connect
      to an external speaker instead of ground. This is ignored for the **V2**
      revision.

.. py:function:: is_playing()

    :returns: ``True`` if audio is playing, otherwise returns ``False``.

.. py:function:: stop()

    Stops all audio playback.


Built-in sounds **V2**
======================

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

Sounds Example
--------------

::

    from microbit import *

    while True:
        if button_a.is_pressed() and button_b.is_pressed():
            # When pressing both buttons only play via the edge connector
            audio.play(Sound.HELLO, pin=pin0)
        elif button_a.is_pressed():
            # On button A play a sound and when it's done show an image
            audio.play(Sound.HAPPY)
            display.show(Image.HAPPY)
        elif button_b.is_pressed():
            # On button B play a sound and show an image at the same time
            audio.play(Sound.TWINKLE, wait=False)
            display.show(Image.BUTTERFLY)

        sleep(500)
        display.clear()


Sound Effects **V2**
====================

.. py:class::
    SoundEffect(freq_start=500, freq_end=2500, duration=500, vol_start=255, vol_end=0, waveform=WAVEFORM_SQUARE, fx=FX_NONE, shape=SHAPE_LOG)

    An ``SoundEffect`` instance represents a sound effect, composed by a set of
    parameters configured via the constructor or attributes.

    All the parameters are optional, with default values as shown above, and
    they can all be modified via attributes of the same name. For example, we
    can first create an effect ``my_effect = SoundEffect(duration=1000)``,
    and then change its attributes ``my_effect.duration = 500``.

    :param freq_start: Start frequency in Hertz (Hz), default: ``500``
    :param freq_end: End frequency in Hertz (Hz), default: ``2500``
    :param duration: Duration of the sound (ms), default: ``500``
    :param vol_start: Start volume value, range 0-255, default: ``255``
    :param vol_end: End volume value, range 0-255, default: ``0``
    :param waveform: Type of waveform shape, one of these values:
        ``WAVEFORM_SINE``, ``WAVEFORM_SAWTOOTH``, ``WAVEFORM_TRIANGLE``,
        ``WAVEFORM_SQUARE``, ``WAVEFORM_NOISE`` (randomly generated noise).
        Default: ``WAVEFORM_SQUARE``
    :param fx: Effect to add on the sound, one of the following values:
        ``FX_TREMOLO``, ``FX_VIBRATO``, ``FX_WARBLE``, or ``FX_NONE``.
        Default: ``FX_NONE``
    :param shape: The type of the interpolation curve between the start and end
        frequencies, different wave shapes have different rates of change
        in frequency. One of the following values: ``SHAPE_LINEAR``,
        ``SHAPE_CURVE``, ``SHAPE_LOG``. Default: ``SHAPE_LOG``

    .. py:function:: copy()

        :returns: A copy of the SoundEffect.

    .. py:attribute:: freq_start

        Start frequency in Hertz (Hz), a number between ``0`` and ``9999``.

    .. py:attribute:: freq_end

        End frequency in Hertz (Hz), a number between ``0`` and ``9999```.

    .. py:attribute:: duration

        Duration of the sound in milliseconds, a number between ``0`` and
        ``9999``.

    .. py:attribute:: vol_start

        Start volume value, a number between ``0`` and ``255``.

    .. py:attribute:: vol_end

        End volume value, a number between ``0`` and ``255``.

    .. py:attribute:: waveform

        Type of waveform shape, one of these values: ``WAVEFORM_SINE``,
        ``WAVEFORM_SAWTOOTH``, ``WAVEFORM_TRIANGLE``, ``WAVEFORM_SQUARE``,
        ``WAVEFORM_NOISE`` (randomly generated noise).

    .. py:attribute:: fx

        Effect to add on the sound, one of the following values:
        ``FX_TREMOLO``, ``FX_VIBRATO``, ``FX_WARBLE``, or ``None``.

    .. py:attribute:: shape

        The type of interpolation curve between the start and end
        frequencies, different wave shapes have different rates of change
        in frequency. One of the following values: ``SHAPE_LINEAR``,
        ``SHAPE_CURVE``, ``SHAPE_LOG``.

The arguments used to create any Sound Effect,
can be inspected by looking at each of the SoundEffect instance attributes,
or by converting the instance into a string (which can be done via ``str()``
function, or by using a function that does the conversion automatically like
``print()``).

For example, with the :doc:`REPL </devguide/repl>` you can inspect the
default SoundEffects::

    >>> print(audio.SoundEffect())
    SoundEffect(freq_start=500, freq_end=2500, duration=500, vol_start=255, vol_end=0, waveform=WAVE_SQUARE, fx=FX_NONE, shape=SHAPE_LOG)

This format is "human readable", which means it is easy for us to read,
and it looks very similar to the code needed to create that SoundEffect,
but it's not quite right. The ``repr()`` function can be used to create a
string of Python code that can be stored or transferred
(you could transmit sounds via micro:bit radio!) and be executed with the
``eval()`` function::

    >>> from audio import SoundEffect
    >>> sound_code = repr(SoundEffect())
    >>> print(sound_code)
    SoundEffect(500, 2500, 500, 255, 0, 3, 0, 18)
    >>> eval("audio.play({})".format(sound_code))

Sound Effects Example
---------------------

.. include:: ../examples/soundeffects.py
    :code: python

AudioFrame
==========

.. py:class::
    AudioFrame

    An ``AudioFrame`` object is a list of 32 samples each of which is an unsigned byte
    (whole number between 0 and 255).

    It takes just over 4 ms to play a single frame.

    .. py:function:: copyfrom(other)

        Overwrite the data in this ``AudioFrame`` with the data from another
        ``AudioFrame`` instance.

        :param other: ``AudioFrame`` instance from which to copy the data.

Technical Details
-----------------

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
32000 cycles in micro:bit V1 or 128000 in V2, so should be plenty).


AudioFrame Example
------------------

.. include:: ../examples/waveforms.py
    :code: python
