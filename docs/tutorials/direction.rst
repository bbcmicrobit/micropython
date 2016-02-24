Direction
---------

There is a compass on the BBC micro:bit. If you ever make a weather station
use the device to work out the wind direction.

Compass
+++++++

It can also tell you the direction of North like this::

    from microbit import *

    compass.calibrate()

    while True:
        needle = ((15 - compass.heading()) // 30) % 12
        display.show(Image.ALL_CLOCKS[needle])

**You must calibrate the compass before taking readings.** Failure to do so
will just produce garbage results. The ``calibration`` method runs a fun little
game to help the device work out where it is in relation to the Earth's
magnetic field.

The program takes the ``compass.heading`` and, using some simple yet
cunning maths (floor division ``//`` and modulo ``%`` ~ look up what these
mean), works out the number of the clock hand to use to display on the screen
so that it is pointing roughly North.
