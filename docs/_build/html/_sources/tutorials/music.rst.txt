Music
-----

MicroPython on the BBC micro:bit comes with a powerful music and sound module.
It's very easy to generate bleeps and bloops from the device *if you attach a
speaker*. Use crocodile clips to attach pin 0 and GND to the positive and
negative inputs on the speaker - it doesn't matter which way round they are
connected to the speaker.

.. image:: pin0-gnd.png

.. note::

    Do not attempt this with a Piezo buzzer - such buzzers are only able to
    play a single tone.

Let's play some music::

    import music

    music.play(music.NYAN)

Notice that we import the ``music`` module. It contains methods used to make
and control sound.

MicroPython has quite a lot of built-in melodies. Here's a complete list:

* ``music.DADADADUM``
* ``music.ENTERTAINER``
* ``music.PRELUDE``
* ``music.ODE``
* ``music.NYAN``
* ``music.RINGTONE``
* ``music.FUNK``
* ``music.BLUES``
* ``music.BIRTHDAY``
* ``music.WEDDING``
* ``music.FUNERAL``
* ``music.PUNCHLINE``
* ``music.PYTHON``
* ``music.BADDY``
* ``music.CHASE``
* ``music.BA_DING``
* ``music.WAWAWAWAA``
* ``music.JUMP_UP``
* ``music.JUMP_DOWN``
* ``music.POWER_UP``
* ``music.POWER_DOWN``

Take the example code and change the melody. Which one is your favourite? How
would you use such tunes as signals or cues?

Wolfgang Amadeus Microbit
+++++++++++++++++++++++++

Creating your own tunes is easy!

Each note has a name (like ``C#`` or ``F``), an octave (telling MicroPython how
high or low the note should be played) and a duration (how
long it lasts through time). Octaves are indicated by a number ~ 0 is the
lowest octave, 4 contains middle C and 8 is about as high as you'll ever need
unless you're making music for dogs. Durations are also expressed as numbers.
The higher the value of the duration the longer it will last. Such
values are related to each other - for instance, a duration of ``4`` will last
twice as long as a duration ``2`` (and so on). If you use the note name ``R``
then MicroPython will play a rest (i.e. silence) for the specified duration.

Each note is expressed as a string of characters like this::

    NOTE[octave][:duration]

For example, ``"A1:4"`` refers to the note named ``A`` in octave number ``1``
to be played for a duration of ``4``.

Make a list of notes to create a melody (it's equivalent to creating an
animation with a list of images). For example, here's how to make MicroPython
play opening of "Frere Jaques"::

    import music

    tune = ["C4:4", "D4:4", "E4:4", "C4:4", "C4:4", "D4:4", "E4:4", "C4:4",
            "E4:4", "F4:4", "G4:8", "E4:4", "F4:4", "G4:8"]
    music.play(tune)

.. note::

    MicroPython helps you to simplify such melodies. It'll remember the octave
    and duration values until you next change them. As a result, the example
    above can be re-written as::

        import music

        tune = ["C4:4", "D", "E", "C", "C", "D", "E", "C", "E", "F", "G:8",
                "E:4", "F", "G:8"]
        music.play(tune)

    Notice how the octave and duration values only change when they have to.
    It's a lot less typing and simpler to read.

Sound Effects
+++++++++++++

MicroPython lets you make tones that are not musical notes. For example, here's
how to create a Police siren effect::

    import music

    while True:
        for freq in range(880, 1760, 16):
            music.pitch(freq, 6)
        for freq in range(1760, 880, -16):
            music.pitch(freq, 6)


Notice how the ``music.pitch`` *method* is used in this instance. It expects a
frequency. For example, the frequency of ``440`` is the same as a concert ``A``
used to tune a symphony orchestra.

In the example above the ``range`` function is used to generate ranges of
numeric values. These numbers are used to define the pitch of the tone. The
three arguments for the ``range`` function are the start value, end value and
step size. Therefore, the first use of ``range`` is saying, in English, "create
a range of numbers between 880 and 1760 in steps of 16". The second use of
``range`` is saying, "create a range of values between 1760 and 880 in steps of
-16". This is how we get a range of frequencies that go up and down in pitch
like a siren.

Because the siren should last forever it's wrapped in an infinite ``while``
loop.

Importantly, we have introduced a new sort of a loop inside the ``while``
loop: the ``for`` loop. In English it's like saying, "for each item in some
collection, do some activity with it". Specifically in the example above, it's
saying, "for each frequency in the specified range of frequencies, play the
pitch of that frequency for 6 milliseconds". Notice how the thing to do for
each item in a for loop is indented (as discussed earlier) so Python knows
exactly which code to run to handle the individual items.
