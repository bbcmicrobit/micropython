API Design Heuristics
*********************

The API should be Pythonic yet pragmatic. We formulated the following heuristics at PyCon UK 2015.

    * If there is only one of a certain type of hardware, we instantiate it and make the resulting object available under the ``microbit`` namespace. E.g. ``microbit.compass``.
    * If there are several of certain types of hardware we enumerate them under a namespace for that type of hardware. E.g. ``microbit.buttons.a``
    * We use short, obvious commonly used names for things and avoid acronyms. Ergo, ``microbit.io`` is renamed to ``microbit.pins``.
    * Pins, because they are objects, are names ``p0``-``p20`` (not uppercase).
    * The ``set_analog_interval`` method is universal in scope for the pins, so is made available under the ``microbit.pins`` namespace.
    * Each pin object will only have methods that match its actual capabilities - so if it's a digital only pin, it shouldn't have analog methods.
    * The pin methods are: ``write_digital(x)``, ``read_digital()``, ``write_analog(x)`` and ``read_analog()``. They're not attributes so they can be used in lambdas and for consistency (everything else is a function).
    * The ``microbit.display`` object has several related methods to draw things on the display in increasing complexity / capability: ``print``, ``scroll`` and ``animate``.
    * The display has two modes: on/off and greyscale. Only 4 or 5 graduations of greyscale are actually visible. We could make these easy to use/remember (0-5???).
    * The ``animate`` method would take the ``wait`` and ``loop`` flags (to simplify the language used).
    * It would be possible to implement a sound / music API using the PWM capabilities of the device. @ntoll to flesh this out in public.
    * We need more built-in images in addition to the 20 or so we already have.
    * Suggested Easter eggs:
        1. ``import love``
        2. ``import antigravity``
        3. "Australia" mode (in honour of Damien - everything on the display is upside down to what it should be)
        4. built in animations for common signals - e.g. the heartbeat we already have
