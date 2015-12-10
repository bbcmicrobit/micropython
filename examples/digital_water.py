"""
Digital Water - your micro water simulator. By Tom Viner

Explanation and see running:
https://www.youtube.com/watch?v=OBTUjoc46Pk
"""

import microbit

# define some constants

DISPLAY_WIDTH = 5
DISPLAY_HEIGHT = 5

MIN_BRIGHTNESS = 0
MEDIUM_BRIGHTNESS = 4
MAX_BRIGHTNESS = 9

# this is how the accelerometer values 1g of gravity
ONE_G = 1024

# Some maths functions to help us

def clamp(minimum, n, maximum):
    """Return the nearest value to n, that's within minimum to maximum (incl)
    """
    return max(minimum, min(n, maximum))

def rescale(src_scale, dest_scale, x):
    """Map one number scale to another

    For example, to convert a score of 4 stars out of 5 into a percentage:
    >>> rescale((0, 5), (0, 100), 4)
    80.0

    Great for mapping different input values into LED pixel brightnesses!
    """
    src_start, src_end = src_scale
    # what proportion along src_scale x is:
    proportion = 1.0 * (x - src_start) / (src_end - src_start)

    dest_start, dest_end = dest_scale
    # apply our proportion to the dest_scale
    return proportion * (dest_end - dest_start) + dest_start

# Helpers for controling the display

def light(brightness, filter):
    """Light up all pixels matching the filter function
    """
    brightness = clamp(MIN_BRIGHTNESS, round(brightness), MAX_BRIGHTNESS)
    for col in range(DISPLAY_WIDTH):
        for row in range(DISPLAY_HEIGHT):
            if filter(col, row):
                microbit.display.set_pixel(col, row, brightness)

def fade_display():
    """Reduce every pixel by 1 brightness level

    This means as we draw new things, the old ones will fade away
    """
    for col in range(5):
        for row in range(5):
            brightness = microbit.display.get_pixel(col, row)
            # reduce by one, but make sure it's still in 0 to 9
            brightness = clamp(MIN_BRIGHTNESS, brightness - 1, MAX_BRIGHTNESS)
            microbit.display.set_pixel(col, row, brightness)


def paint_water():
    """Use the accelerometer to paint a water level on the display
    """
    # read the current orientation values from the accelerometer
    X, Y, Z = microbit.accelerometer.get_values()

    # map the force in the X-axis to a turn factor from -2 to 2
    # -ONE_G is button A at the top, ONE_G is button B at the top
    turn_factor = rescale((-ONE_G, ONE_G), (-2, 2), X)

    # map the force in the Z-axis to a spill factor from -3 to 3
    # this allows the water to cover the whole display when it's flat
    spill_factor = rescale((ONE_G, -ONE_G), (-3, 3), Z)

    # use the variables above to make a filter function, customised for the
    # current orientation of the micro:bit
    def filter(col, row):
        """For a given pixel position, decide if it should be on or not
        """
        if Y < 0:
            # we're upside down, so reverse the y-axis value
            # (- 1 because we start counting rows from 0, not 1)
            row = DISPLAY_HEIGHT - 1 - row
        # remember rows count down from the top, so we want to light up all
        # the rows below the water line (when the micro:bit is help up straight)
        # The forumula here is of the form y = m*x + c
        # We have a couple of "- 2"s to centre the water level in the middle 
        # of the display
        return row - 2 > -turn_factor * (col - 2) - spill_factor

    # we want the water to "dilute" when spread out across the whole display
    overall_brightness = rescale(
        (0, ONE_G),
        (MAX_BRIGHTNESS, MEDIUM_BRIGHTNESS),
        abs(Z)
    )

    # light up the pixels when filter returns true, to the given bright level
    light(overall_brightness, filter)

# loop forever painting watery pixels, sleeping and then fading as each pixel
# washes away into the night

while True:
    paint_water()

    microbit.sleep(100)

    # fade all pixels by one brightness level
    fade_display()
