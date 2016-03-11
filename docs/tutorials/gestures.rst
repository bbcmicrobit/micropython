Gestures
--------

The really interesting side-effect of having an accelerometer is gesture
detection. If you move your BBC micro:bit in a certain way (as a gesture) then
MicroPython is able to detect this.

MicroPython is able to recognise the following gestures: ``up``, ``down``,
``left``, ``right``, ``face up``, ``face down``, ``freefall``, ``3g``, ``6g``,
``8g``, ``shake``. Gestures are always represented as strings. While most of
the names should be obvious, the ``3g``, ``6g`` and ``8g`` gestures apply when
the device encounters these levels of g-force (like when an astronaut is
launched into space).

To get the current gesture use the ``accelerometer.current_gesture`` method.
Its result is going to be one of the named gestures listed above. For example,
this program will only make your device happy if it is face up::

    from microbit import *

    while True:
        gesture = accelerometer.current_gesture()
        if gesture == "face up":
            display.show(Image.HAPPY)
        else:
            display.show(Image.ANGRY)

Once again, because we want the device to react to changing circumstances we
use a ``while`` loop. Within the *scope* of the loop the current gesture is
read and put into ``gesture``. The ``if`` conditional checks if ``gesture`` is
equal to ``"face up"`` (Python uses ``==`` to test for equality, a single
equals sign ``=`` is used for assignment - just like how we assign the gesture
reading to the ``gesture`` object). If the gesture is equal to ``"face up"``
then use the display to show a happy face. Otherwise, the device is made to
look angry!

Magic-8
+++++++

A Magic-8 ball is a toy first invented in the 1950s. The idea is to ask
it a yes/no question, shake it and wait for it to reveal the truth. It's rather
easy to turn into a program::

    from microbit import *
    import random

    answers = [
        "It is certain",
        "It is decidedly so",
        "Without a doubt",
        "Yes, definitely",
        "You may rely on it",
        "As I see it, yes",
        "Most likely",
        "Outlook good",
        "Yes",
        "Signs point to yes",
        "Reply hazy try again",
        "Ask again later",
        "Better not tell you now",
        "Cannot predict now",
        "Concentrate and ask again",
        "Don't count on it"
        "My reply is no",
        "My sources say no",
        "Outlook not so good",
        "Very doubtful",
    ]

    while True:
        display.show("8")
        if accelerometer.was_gesture("shake"):
            display.clear()
            sleep(1000)
            display.scroll(random.choice(answers))

Most of the program is a list called ``answers``. The actual game is in the
``while`` loop at the end.

The default state of the game is to show the character ``"8"``. However, the
program needs to detect if it has been shaken. The ``was_gesture`` method uses
its argument (in this case, the string ``"shake"`` because we want to detect
a shake) to return a ``True`` / ``False`` response. If the device was shaken
the ``if`` conditional drops into its block of code where it clears the screen,
waits for a second (so the device appears to be thinking about your question)
and displays a randomly chosen answer.

Why not ask it if this is the greatest program ever written? What could you do
to "cheat" and make the answer always positive or negative? (Hint: use the
buttons.)
