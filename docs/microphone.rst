Microphone **V2**
*****************

.. py::module:: microbit

This object lets you access the on-board microphone available on the **V2**
micro:bit version. It can be used to respond to sound. The microphone input
is located on the front of the board alongside a microphone activity LED,
which is lit when the microphone is in use.

.. image:: microphone.png
    :width: 300px
    :align: center
    :height: 240px
    :alt: micro:bit with microphone LED on

Sound events
============
The microphone can respond to a pre-defined set of sound events that are
based on the amplitude and wavelength of the sound.

These sound events are identified by strings passed as method arguments and
return values. These strings are also stored as `MicrobitMicrophone` class
variables as a handy way to identify what events are available.

Classes
=======
.. py:class:: MicrobitMicrophone

    .. note::
        The ``MicrobitMicroPhone`` class documented here can be accessed via 
        the microbit module as ``microbit.microphone``. It cannot be
        initialised on it's own.

    Represents the microphone.

.. py:class:: SoundEvent  

    Value to represent loud sound events, like clapping or shouting
    ``SoundEvent.LOUD`` = ``loud``.

    Value to represent quiet sound events, like speaking or background music
    ``SoundEvent.QUIET`` = ``quiet``.

.. py:function:: current_event()

    Returns the name of the last recorded sound event, ``loud`` or ``quiet``.

.. py:function:: was_event(SoundEvent.LOUD)

    Parameter: A sound event,  such as ``SoundEvent.LOUD`` or ``SoundEvent.QUIET``.
    
    Returns ``true`` if sound was heard at least once since the last call,
    otherwise ``false``.

.. py:function:: get_events():

    Returns a tuple of the event history. The most recent is listed last.

    Also clears the sound event history before returning.

.. py:function:: set_threshold()

    Parameter: A sound event, such as ``SoundEvent.LOUD`` or ``SoundEvent.QUIET``.
    
    Parameter: The threshold level in the range 0-255.
    
    For example, ``set_threshold(SoundEvent.LOUD, 250)`` will only trigger if
    the sound is very loud (>= 250).

.. py:function:: sound_level()

    Returns a representation of the sound pressure level in the range 0 to
    255.


Example
=======

An example that runs through some of the functions of the microphone API

.. include:: ../examples/microphone.py
    :code: python