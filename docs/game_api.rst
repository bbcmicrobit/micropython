Game API
********

.. py:module:: game

This is documentation for the game API - based upon the amazing work done by
Larry Hastings. THIS IS A STRAW MAN.


Overview
========

Writing games is a wonderful way to introduce programming to beginners. We aim
to make this as simple as possible given the hardware constraints of the BBC
micro:bit. As Larry says,

  Using [the micro:bit] properly can require clever code and programming
  insights far beyond what is reasonable to expect from the micro:bit's
  beginner programmers. Not to mention, the code size for these solutions
  would eat into the astonishingly restrictive RAM space available on the
  micro:bit.

Wise words indeed. Larry's excellent primer and discussion of his work can be
found here:

https://bitbucket.org/larry/microbit_game

The purpose of this document is to outline the three classes as imagined in
the microbit API. It's currently a straw man that we can poke with a stick. The
intention is to create an API suitable for eleven year old kids so much of my
re-write / re-naming is done with this in mind. PLEASE FEEL FREE TO CHANGE.

There are three classes (each described below) found in the `microbit.game`
namespace:

* `Controller` (was accelerometer_scaler_new) - simplifies the reading of the accelerometer so that it is appropriate for controlling games.
* `Trigger` (was button_minder) - easy reading of button states.
* `Scheduler` - a very simple scheduler to trigger things after some pre-defined period of time.

Controller
==========

The micro:bit would be great for controlling a character or scrolling the
screen. This class allows kids to define a range of desired numeric states to
be output as the device is moved along a certain axis.

The class introduces "dead zones" to ensure tremours in movement don't result
in wildly changing values.

It's also possible to reverse the range by setting the upper bound as the
output minimum and the lower bound as the output maximum.

Finally, it's possible to set the size of the middle of the range. As Larry
explains,

  If you were simulating a lever balancing on a fulcrum, you might want a
  large range to mean 0 (hold steady), and small ranges on the ends to mean
  "tilt left" (-1) and "tilt right" (+1).

.. py:class:: Controller(read, accel_min=512, accel_max=512, out_min=-2, out_max=2, middle=200, deadzone=0.2)

    Creates a controller object with the ``read`` function that takes no
    arguments and returns an integer. This function is called to get the
    current value of the accelerometer.

    ``accel_min`` defines the minimum value reported by the ``read`` function
    that the object should accept. Values below this will be clipped to
    ``accel_min``.

    ``accel_max`` defines the maximum value reported by the ``read`` function
    that the object should accept. Values above this will be clipped to
    ``accel_max``.

    ``out_min`` defines the minimum value the object should scale to. The
    object's ``scan`` method will never report a number lower than this.

    ``out_max`` defines the maximum value the object should scale to. The
    object's ``scan`` method will never report a number higher than this.

    ``middle`` defines the amount to pad the centre region of the input in
    terms of the accelerometer's range. The non-US "centre" is not used to
    avoid spelling confusion.

    ``deadzone`` represents how big the dead zones are, expressed as a floating
    point number relating to the "live" zones. The default value of 0.2 makes
    the dead zones 20% of the size of the live zones.

    .. py:method:: scan()

    Scans the current state returned by the ``read`` function. Returns the
    current scaled value.

Trigger
=======

The micro:bit has two buttons: A and B. This class makes it easy to track
their state so they can be used to trigger events and actions in games.

.. py:class:: Trigger(a, b, delay)

    Creates a trigger object. One of each of the parameters must be used. If
    ``a`` is set to ``True`` then the object will monitor button A. If ``b``
    is set to ``True`` then the object will monitor button B. If the ``delay``
    argument is set to an integer indicating milliseconds then the object will
    monitor both buttons if both buttons are pressed and held within ``delay``
    period of time.

    The ``delay`` argument permits us to hit both buttons simultaneously. Since
    we are notoriously slower than computers it's likely that there may be a
    slight ``delay`` between pressing A and pressing B.

    .. py:method:: pressed()

    Returns ``True`` if the button state monitored by the object (defined when
    initialising - see above) is pressed (either, A or B or both). Otherwise,
    returns ``False``.

Scheduler
=========

Often people will want to do something, pause for a while, then do other
things. In other words, they'll need a way to schedule things to happen in the
future. This class is similar to the scheduler class in Python's sched module
found in the standard library.

Users add events (represented by arbitrary values) and time delays to the
schedule. Run the schedule to pause and, at the appropriate moment, get the
next scheduled event.

.. py:class:: Scheduler

    Creates an object to be used to schedule events after pre-defined periods
    of time.

    .. py:method:: add(event, delta, repeat=False)

    Add an ``event`` to the schedule object to be fired at ``delta``
    milliseconds in the future. If ``repeat`` is ``True`` then this event will
    continue to fire after ``delta`` amount of time. If ``repeat`` is ``False``
    the event will only fire once.

    Returns ``True`` if the ``event`` was added to the schedule.

    .. py:method:: cancel(event)

    Cancels all the events from the schedule identified by ``event``. Returns
    the number of matching events cancelled.

    .. py:method:: clear()

    Clears all the events from the schedule. Returns the number of events
    cancelled.

    .. py:method:: run()

    Runs the scheduler. The program blocks until it's time for the next
    event to fire. The return value (at the time when the event fires) is the
    arbitrary ``event`` value passed in to the ``add`` method when the event
    was created.

    Re-run to get the next scheduled event at the expected time.

    Running the scheduler with an empty schedule is an error. It will raise an
    as-yet-to-be determined exceptioni (IndexError - the schedule is perhaps a
    sort of sequence?).

Here's a made up example of the scheduler in use::

    schedule = game.Scheduler()
    events = {
        'b': 5000,
        'a': 1000,
        'c': 6000,
    }
    for k, v in events.items():
        schedule.add(k, v)
    while True:
        try:
            ev = schedule.run()  # blocks until the next scheduled event
            print(ev)
        except IndexError:
            break

This should output::

    a
    b
    c

...after 1, 5 and 6 seconds respectively.
