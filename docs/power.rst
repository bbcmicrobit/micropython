Power Management **V2**
***********************

.. py:module:: power

This module lets you manage the power modes of the micro:bit V2.

There are two micro:bit board low power modes that can be requested from
MicroPython:

- **Deep Sleep**: Low power mode where the board can be woken up via
  multiple sources (pins, button presses, or a timer) and resume
  operation.
- **Off**: The power mode with the lowest power consumption, the only way to
  wake up the board is via the reset button, or by plugging the USB cable while
  on battery power.
  When the board wakes up it will restart and execute your programme from the
  beginning.

The micro:bit board contains two microcontrollers (MCUs), which can be
independently asleep or awake:

- **Target MCU** - Where MicroPython and your code run.
- **Interface MCU** - A secondary microcontroller that provides the USB
  functionality, like the ``MICROBIT`` USB drive, and the USB serial interface.

.. image:: power-mcus.png

Each MCU can be in one of these "MCU power modes":

- **Awake**: Running normally.
- **Sleep**: A low power mode where the MCU can be woken up from different
  sources and continue operation.
- **Power Down**: The lowest power mode for an individual MCU, when it wakes up
  it will start from reset.

The Python code can request a "board power mode", which will
set the Target into a specific "MCU power mode",
but the Interface MCU mode will depend on the micro:bit power source,
i.e. if it's powered via USB or battery.

+------------------+-----------------+--------------------+
| .. centered:: USB Powered (Interface always awake)      |
+------------------+-----------------+--------------------+
| Board Power Mode | Target MCU Mode | Interface MCU mode |
+==================+=================+====================+
| **Deep Sleep**   | 💤  Sleep       | ⏰ Awake           |
+------------------+-----------------+--------------------+
| **Off**          | 📴 Power Down   | ⏰ Awake           |
|                  |                 | (red LED blinking) |
+------------------+-----------------+--------------------+

+------------------+-----------------+--------------------+
| .. centered:: Battery Powered                           |
+------------------+-----------------+--------------------+
| Board Power Mode | Target MCU Mode | Interface MCU mode |
+==================+=================+====================+
| **Deep Sleep**   | 💤 Sleep        | 💤 Sleep           |
+------------------+-----------------+--------------------+
| **Off**          | 📴 Power Down   | 📴 Power Down      |
+------------------+-----------------+--------------------+

Functions
=========

.. py:function:: off()

    Power down the board to the lowest possible power mode.

    This is the equivalent to pressing the reset button for a few second,
    to set the board in "Off mode".

    The Target MCU will go into Power Down mode. The Interface MCU will go to
    Power Down on battery or stay awake on USB, where it will blink the red
    LED near the USB connector.

    The micro:bit will only wake up if the reset button is pressed or,
    if on battery power, when a USB cable is connected.

    When the board wakes up it will start for a reset state, so your programme
    will start running from the beginning.


.. py:function:: deep_sleep(ms=None, pins=None, buttons=None, run_every=False)

    Set the micro:bit into a low power mode where it can wake up and continue
    operation.

    The programme state is preserved and when it wakes up it will resume
    operation where it left off.

    Deep Sleep mode will consume more battery power than Off mode.

    The wake up sources are configured via arguments.

    If no wake up sources have been configured it will sleep until the reset
    button is pressed (which resets the Target MCU) or, in battery power,
    when the USB cable is inserted.

    :param ms: A time in milliseconds to wait before it wakes up.
    :param pins: A single instance or a tuple of pins, e.g.
        ``deep_sleep(pins=(pin0, pin2))``.
    :param buttons: A single instance or a tuple of buttons, e.g.
        ``deep_sleep(buttons=button_a)``.
    :param run_every: Set to ``True`` to wake up with each
        ``microbit.run_every`` scheduled run.

Examples
========

Example programme showing the power management API::

    """
    Shows a silly face on the display every 20 seconds.
    When button B is pressed it goes into Deep Sleep mode, and wakes 5 minutes later.
    When button A is pressed it goes into to Off mode.
    """
    from microbit import *

    @run_every(s=20)
    def silly_face():
        display.show(Image.SILLY)
        sleep(500)

    while True:
        if button_a.is_pressed():
            display.scroll("Off")
            # In this mode the micro:bit can only wake up via the reset button
            power.off()
            # This line of code will never be executed, as waking up from this
            # mode starts the programme from the beginning
            display.show(Image.SURPRISED)
        elif button_b.is_pressed():
            display.scroll("Sleep")
            # Go into Deep Sleep with multiple wake up sources
            power.deep_sleep(
                pins=(pin0, pin1),
                buttons=button_a,
                ms=5*60*1000,      # In 5 minutes it wakes up anyway
                run_every=False,   # Blocks run_every from waking up the board
            )
            # When the micro:bit wakes up will it continue running from here
            display.show(Image.ASLEEP)
            sleep(1000)
        display.show(Image.HAPPY)
        sleep(200)


Example using data logging::

    from microbit import *
    import log

    # Log the temperature every 5 minutes
    @run_every(min=5)
    def log_temperature():
        log.add(temp=temperature())

    while True:
        if button_a.is_pressed():
            # Display the temperature when button A is pressed
            display.scroll(temperature())
        # To go sleep and wake up with run_every or button A
        power.deep_sleep(buttons=button_a, run_every=True)
