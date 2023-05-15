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

TODO:
* Describe the feature.
* Indicate how the sampling rate relates to recording quality.
* Indicate how changing the sampling rate on the fly affects playback speed.
* What happens if the user changes the sampling rate while recording?

::

    from microbit import *

    while True:
        if button_a.is_pressed():
            my_recording = microphone.record(duration=5000)
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

.. py:function:: record(duration=3000, rate=11000, wait=True)

    Record sound for the amount of time indicated by ``duration`` at the
    sampling rate indicated by ``rate``.

    The amount of memory consumed is directly related to the length of the
    recording and the sampling rate. The higher these values, the more memory
    it will use.

    A lower sampling rate will reduce memory consumption and sound quality.

    If there isn't enough memory available a ``MemoryError`` will be raised.

    :param duration: How much time to record in milliseconds.
    :param rate: Number of samples to capture per second.
    :param wait: When set to ``True`` it blocks until the recording is
        done, if it is set to ``False`` it will run in the background.
    :returns: An ``AudioBuffer``, configured at the provided ``duration``
        and ``rate``, with the sound data.

.. py:function:: record_into(buffer, rate=11000, wait=True)

    Record sound into an existing ``AudioBuffer``.

    :param buffer: An ``AudioBuffer`` to record the microphone sound.
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
    constants are valid arguments

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

    talk_open = Image(
        "09090:"
        "00000:"
        "09990:"
        "90009:"
        "09990"
    )
    talk_closed = Image(
        "09090:"
        "00000:"
        "00000:"
        "99999:"
        "00000"
    )

    my_recording = audio.AudioBuffer(duration=5000, rate=5500)

    while True:
        if button_a.is_pressed():
            microphone.record_into(my_recording, rate=5500, wait=False)
            display.show([talk_open, talk_closed], loop=True, wait=False, delay=150)
            while button_a.is_pressed():
                sleep(50)
            display.show(mouth_open, loop=False) # workaround issue #150
            display.clear()
        if button_b.is_pressed():
            audio.play(my_recording, wait=False)
            while audio.is_playing():
                x = accelerometer.get_x()
                my_recording.rate = scale(x, (-1000, 1000), (2250, 11000))
                sleep(50)
        sleep(100)
