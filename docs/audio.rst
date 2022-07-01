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

3. `Audio Frames <##audioframe>`_, an iterable (like a list or a generator)
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
    SoundEffect(preset=None, freq_start=400, freq_end=200, duration=500, vol_start=100, vol_end=255, wave=WAVE_SQUARE, fx=None, interpolation=INTER_LINEAR)

    An ``SoundEffect`` instance represents a sound effect, composed by a set of
    parameters configured via the constructor or attributes.

    All the parameters are optional, with default values as shown above, and
    they can all be modified via attributes of the same name. For example, we
    can first create an effect ``my_effect = SoundEffect(duration=1000)``,
    and then change its attributes ``my_effect.duration = 500``.

    :param preset: An existing SoundEffect instance to use as a base, its values
        are cloned in the new instance, and any additional arguments provided
        overwrite the base values.
    :param freq_start: Start Frequency in Hertz (Hz), eg: ``400``
    :param freq_end: End Frequency in Hertz (Hz), eg: ``2000``
    :param duration: Duration of the sound (ms), eg: ``500``
    :param vol_start: Start volume value, range 0-255, eg: ``120``
    :param vol_end: End volume value, range 0-255, eg: ``255``
    :param wave: Type of wave shape, one of these values: ``WAVE_SINE``,
        ``WAVE_SAWTOOTH``, ``WAVE_TRIANGLE``, ``WAVE_SQUARE``,
        ``WAVE_NOISE`` (randomly generated noise).
    :param fx: Effect to add on the sound, one of the following values:
        ``FX_TREMOLO``, ``FX_VIBRATO``, ``FX_WARBLE``, or ``None``.
    :param interpolation: The type of curve between the start and end
        frequencies, different wave shapes have different rates of change
        in frequency. One of the following values: ``INTER_LINEAR``,
        ``INTER_CURVE``, ``INTER_LOG``.

    .. py:attribute:: freq_start

        Start Frequency in Hertz (Hz)

    .. py:attribute:: freq_end

        End Frequency in Hertz (Hz)

    .. py:attribute:: duration

        Duration of the sound (ms), eg: ``500``

    .. py:attribute:: vol_start

        Start volume value, range 0-255, eg: ``120``

    .. py:attribute:: vol_end

        End volume value, range 0-255, eg: ``255``

    .. py:attribute:: wave

        Type of wave shape, one of these values: ``WAVE_SINE``,
        ``WAVE_SAWTOOTH``, ``WAVE_TRIANGLE``, ``WAVE_SQUARE``,
        ``WAVE_NOISE`` (randomly generated noise).

    .. py:attribute:: fx

        Effect to add on the sound, one of the following values:
        ``FX_TREMOLO``, ``FX_VIBRATO``, ``FX_WARBLE``, or ``None``.

    .. py:attribute:: interpolation

        The type of curve between the start and end
        frequencies, different wave shapes have different rates of change
        in frequency. One of the following values: ``INTER_LINEAR``,
        ``INTER_CURVE``, ``INTER_LOG``.

The arguments used to create any Sound Effect, including the built in ones,
can be inspected by looking at each of the SoundEffect instance attributes,
or by converting the instance into a string (which can be done via ``str()``
function, or by using a function that does the conversion automatically like
``print()``).

For example, with the :doc:`REPL </devguide/repl>` you can inspect the built
in Effects::

    >>> audio.SoundEffect.CROAK
    SoundEffect(freq_start=..., freq_end=..., duration=..., vol_start=..., vol_end=..., wave=..., fx=..., interpolation=...)

The built in Effects are immutable, so they cannot be changed. Trying to modify
a built in SoundEffect will throw an exception::

    >>> audio.SoundEffect.CLICK.duration = 1000
    Traceback (most recent call last):
        File "<stdin>", line 1, in <module>
    TypeError: effect cannot be modified

But a new one can be created like this::

    >>> click_clone = SoundEffect(audio.SoundEffect.CLICK)
    >>> click_clone.duration = 1000
    >>>

Built in Sound Effects
----------------------

Some pre-created Sound Effects are already available as examples. These can
be played directly ``audio.play(audio.SoundEffect.SQUEAK)``,
or used as a base to create new effects
``audio.SoundEffect(audio.SoundEffect.SQUEAK, duration=2000)``.

* ``audio.SoundEffect.SQUEAK``
* ``audio.SoundEffect.WARBLE``
* ``audio.SoundEffect.CHIRP``
* ``audio.SoundEffect.CROAK``
* ``audio.SoundEffect.CLICK``

Sound Effects Example
---------------------

::

    from microbit import *

    # Play a built in Sound Effect
    audio.play(audio.SoundEffect.CHIRP)

    # Create a Sound Effect and immediately play it
    audio.play(audio.SoundEffect(
        freq_start=400,
        freq_end=2000,
        duration=500,
        vol_start=100,
        vol_end=255,
        wave=audio.WAVE_TRIANGLE,
        fx=audio.FX_VIBRATO,
        interpolation=audio.LOG
    ))

    # Play a Sound Effect instance, modify an attribute, and play it again
    my_effect = audio.SoundEffect(
        preset=audio.CHIRP
        freq_start=400,
        freq_end=2000,
    )
    audio.play(my_effect)
    my_effect.duration = 1000
    audio.play(my_effect)

    # You can also create a new effect based on an existing one, and modify
    # any of its characteristics via arguments
    audio.play(audio.SoundEffect.WARBLE)
    my_modified_effect = SoundEffect(audio.SoundEffect.WARBLE, duration=1000)
    audio.play(my_modified_effect)

    # Use sensor data to modify and play the existing Sound Effect instance
    while True:
        my_effect.freq_start=accelerometer.get_x()
        my_effect.freq_end=accelerometer.get_y()
        audio.play(my_effect)

        if button_a.is_pressed():
            # On button A play an effect and once it's done show an image
            audio.play(audio.SoundEffect.CHIRP)
            display.show(Image.DUCK)
            sleep(500)
        elif button_b.is_pressed():
            # On button B play an effect while showing an image
            audio.play(audio.SoundEffect.CLICK, wait=False)
            display.show(Image.SQUARE)
            sleep(500)


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
32k cycles in micro:bit V1 or 128k in V2, so should be plenty).


AudioFrame Example
------------------

.. include:: ../examples/waveforms.py
    :code: python
