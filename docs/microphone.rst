Microphone **V2**
*****************

.. py:module:: microbit.microphone

This object lets you access the built-in microphone available on the
micro:bit **V2**. It can be used to record and respond to sound.
The microphone input is located on the front of the board alongside a
microphone activity LED, which is lit when the microphone is in use.

.. image:: microphone.png
    :width: 300px
    :align: center
    :height: 240px
    :alt: micro:bit with microphone LED on

Sound events
============
The microphone can respond to a pre-defined set of sound events that are
based on the amplitude and wavelength of the sound.

These sound events are represented by instances of the ``SoundEvent`` class,
accessible via variables in ``microbit.SoundEvent``:

- ``microbit.SoundEvent.QUIET``: Represents the transition of sound events,
  from ``loud`` to ``quiet`` like speaking or background music.

- ``microbit.SoundEvent.LOUD``: Represents the transition of sound events,
  from ``quiet`` to ``loud`` like clapping or shouting.

Recording
=========

The microphone can record audio into an :doc:`AudioFrame <audio>`, which can
then be played with the ``audio.play()`` function.

Audio sampling is the process of converting sound into a digital format.
To do this, the microphone takes samples of the sound waves at regular
intervals. The number of samples recorded per second is known as the
"sampling rate", so recording at a higher sampling rate increases the sound
quality, but as more samples are saved, it also consumes more memory.

The microphone sampling rate can be configured during sound recording via
the ``AudioFrame.rate()`` method functions.

At the other side, the audio playback sampling rate indicates how many samples
are played per second. So if audio is played back with a higher sampling rate
than the rate used during recording, then the audio will sound speeded up.
If the playback sampling rate is twice the recording rate, the sound will take
half the time to be played to completion. Similarly, if the playback rate
is halved, it will play half as many samples per second, and so it will
take twice as long to play the same amount of samples.

How do you think a voice recording will sound if the playback rate is
increased or decreased? Let's try it out!::

    from microbit import *

    while True:
        if pin_logo.is_touched():
            # Record and play back at the same rate
            my_recording = microphone.record(duration=3000)
            audio.play(my_recording)

        if button_a.is_pressed():
            # Play back at half the sampling rate
            my_recording = microphone.record(duration=3000)
            my_recording.set_rate(my_recording.get_rate() // 2)
            audio.play(my_recording)

        if button_b.is_pressed():
            # Play back at twice the sampling rate
            my_recording = microphone.record(duration=3000)
            my_recording.set_rate(my_recording.get_rate() * 2)
            audio.play(my_recording)

        sleep(200)

Functions
=========

.. py:function:: current_event()

    Get the last recorded sound event.

    :return: The event, ``SoundEvent('loud')`` or ``SoundEvent('quiet')``.

.. py:function:: was_event(event)

    Check if a sound was heard at least once since the last call.

    This call clears the sound history before returning.

    :param event: The event to check for,  such as ``SoundEvent.LOUD`` or
        ``SoundEvent.QUIET``.
    :return: ``True`` if sound was heard at least once since the last call,
        otherwise ``False``.

.. py:function:: is_event(event)

    Check the most recent sound event detected.

    This call does not clear the sound event history.

    :param event: The event to check for,  such as ``SoundEvent.LOUD`` or
        ``SoundEvent.QUIET``
    :return: ``True`` if sound was the most recent heard, ``False`` otherwise.

.. py:function:: get_events()

    Get the sound event history as a tuple.

    This call clears the sound history before returning.

    :return: A tuple of the event history with the most recent event last.

.. py:function:: set_threshold(event, value)

    Set the threshold for a sound event.

    The ``SoundEvent.LOUD`` event will be triggered when the sound level
    crosses this threshold upwards (from "quiet" to "loud"),
    and ``SoundEvent.QUIET`` event is triggered when crossing the threshold
    downwards (from "loud" to "quiet").

    If the ``SoundEvent.LOUD`` value set is lower than ``SoundEvent.QUIET``,
    then "quiet" threshold will be decreased to one unit below the "loud"
    threshold. If the ``SoundEvent.QUIET`` value is set higher than
    ``SoundEvent.LOUD``, then the "loud" threshold will be set one unit above.

    :param event: A sound event, such as ``SoundEvent.LOUD`` or
        ``SoundEvent.QUIET``.
    :param value: The threshold level in the range 0-255. Values outside this
        range will be clamped.

.. py:function:: sound_level()

    Get the sound pressure level.

    :return: A representation of the sound pressure level in the range 0 to 255.

.. py:function:: record(duration=3000, rate=7812)

    Record sound into an ``AudioFrame`` for the amount of time indicated by
    ``duration`` at the sampling rate indicated by ``rate``.

    The amount of memory consumed is directly related to the length of the
    recording and the sampling rate. The higher these values, the more memory
    it will use.

    A lower sampling rate will reduce both memory consumption and sound
    quality.

    If there isn't enough memory available a ``MemoryError`` will be raised.

    :param duration: How long to record in milliseconds.
    :param rate: Number of samples to capture per second.
    :returns: An ``AudioFrame`` with the sound samples.

.. py:function:: record_into(buffer, rate=7812, wait=True)

    Record sound into an existing ``AudioFrame`` until it is filled,
    or the ``stop_recording()`` function is called.

    :param buffer: An ``AudioFrame`` to record sound.
    :param rate: Number of samples to capture per second.
    :param wait: When set to ``True`` it blocks until the recording is
        done, if it is set to ``False`` it will run in the background.

.. py:function:: is_recording()

    :returns: ``True`` if the microphone is currently recording sound, or
      ``False`` otherwise.

.. py:function:: stop_recording()

    Stops an a recording running in the background.

.. py:function:: set_sensitivity(gain)

    Configure the microphone sensitivity to one of these three levels:
    ``microphone.SENSITIVITY_LOW``, ``microphone.SENSITIVITY_MEDIUM``,
    ``microphone.SENSITIVITY_HIGH``.

    These constants correspond to a number, and any values between these
    constants are valid arguments.

    :param gain: Microphone gain.

Examples
========

An example that runs through some of the functions of the microphone
Sound Events API::

    # Basic test for microphone.  This test should update the display when
    # Button A is pressed and a loud or quiet sound *is* heard, printing the
    # results. On Button B this test should update the display when a loud or
    # quiet sound *was* heard, printing the results. On shake this should print
    # the last sounds heard, you should try this test whilst making a loud sound
    # and a quiet one before you shake.

    from microbit import *

    display.clear()
    sound = microphone.current_event()

    while True:
        if button_a.is_pressed():
            if microphone.current_event() == SoundEvent.LOUD:
                display.show(Image.SQUARE)
                uart.write('isLoud\n')
            elif microphone.current_event() == SoundEvent.QUIET:
                display.show(Image.SQUARE_SMALL)
                uart.write('isQuiet\n')
            sleep(500)
        display.clear()
        if button_b.is_pressed():
            if microphone.was_event(SoundEvent.LOUD):
                display.show(Image.SQUARE)
                uart.write('wasLoud\n')
            elif microphone.was_event(SoundEvent.QUIET):
                display.show(Image.SQUARE_SMALL)
                uart.write('wasQuiet\n')
            else:
                display.clear()
            sleep(500)
        display.clear()
        if accelerometer.was_gesture('shake'):
            sounds = microphone.get_events()
            soundLevel = microphone.sound_level()
            print(soundLevel)
            for sound in sounds:
                if sound == SoundEvent.LOUD:
                    display.show(Image.SQUARE)
                elif sound == SoundEvent.QUIET:
                    display.show(Image.SQUARE_SMALL)
                else:
                    display.clear()
                print(sound)
                sleep(500)


An example of recording and playback with a display animation::

    from microbit import *

    mouth_open = Image(
        "09090:"
        "00000:"
        "09990:"
        "90009:"
        "09990"
    )
    mouth_closed = Image(
        "09090:"
        "00000:"
        "00000:"
        "99999:"
        "00000"
    )

    RECORDING_RATE = 3906
    RECORDING_MS = 5000

    my_recording = audio.AudioBuffer(duration=RECORDING_MS, rate=RECORDING_RATE)

    while True:
        if button_a.is_pressed():
            microphone.record_into(my_recording, rate=RECORDING_RATE, wait=False)
            display.show([mouth_open, mouth_closed], loop=True, wait=False, delay=150)
            while button_a.is_pressed() and microphone.is_recording():
                sleep(50)
            microphone.stop_recording()
            display.clear()
        if button_b.is_pressed():
            audio.play(my_recording, wait=False)
            while audio.is_playing():
                x = accelerometer.get_x()
                audio.set_rate(scale(x, (-1000, 1000), (2250, 11000)))
                sleep(50)
        sleep(100)
