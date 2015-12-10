"""Four buttons - 2 buttons + 2 more "buttons"!
A little example of using pins 1 & 2 an extra 2 buttons. By Tom Viner

Explanation and see running:
https://www.youtube.com/watch?v=6ofUJ6Mgk4k
"""
import microbit

# define some constants

DISPLAY_WIDTH = 5
DISPLAY_HEIGHT = 5

MIN_BRIGHTNESS = 0
MAX_BRIGHTNESS = 9

# A maths functions to help us

def clamp(minimum, n, maximum):
    """Return the nearest value to n, that's within minimum to maximum (incl)
    """
    return max(minimum, min(n, maximum))


# Helpers for controling the display

def light(brightness, filter):
    """Light up all pixels matching the filter function
    """
    brightness = clamp(MIN_BRIGHTNESS, round(brightness), MAX_BRIGHTNESS)
    for col in range(DISPLAY_WIDTH):
        for row in range(DISPLAY_HEIGHT):
            if filter(col, row):
                microbit.display.set_pixel(col, row, brightness)

def light_column(column):
    """Light up a whole column to max brightness
    """
    def filter_column(col, row):
        """For a given pixel position, turn on if it matches our column
        """
        return col == column
    light(MAX_BRIGHTNESS, filter_column)

def light_row(row):
    """Light up a whole row to max brightness
    """
    def filter_row(col, rw):
        """For a given pixel position, turn on if it matches our row
        """
        return rw == row
    light(MAX_BRIGHTNESS, filter_row)

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

def paint_box(top=0, bottom=DISPLAY_HEIGHT-1, left=0, right=DISPLAY_WIDTH-1):
    """Draw a filled in rectangle on the display
    """
    def filter_box(col, row):
        """For a given pixel position, turn it on if it's with the bounds
        """
        # remember rows count from 0 at the top!
        correct_vertical = top <= row <= bottom
        correct_horizontal = left <= col <= right
        return correct_vertical and correct_horizontal
    light(MAX_BRIGHTNESS, filter_box)


# Our main functions

def pin_is_touched(n):
    """Pass in a pin number (1 or 2),
    get back True if it's being touched right now

    In this way, it acts just like microbit.button_a.is_pressed()
    """
    pin = getattr(microbit, 'pin{}'.format(n))
    return pin.read_analog() > 300

def four_buttons():
    """Push buttons, touch pins, see if you can light up the whole display!
    """
    a_is_pressed = microbit.button_a.is_pressed()
    b_is_pressed = microbit.button_b.is_pressed()

    # let's call the two pin-buttons c and d:
    c_is_pressed = pin_is_touched(1)
    d_is_pressed = pin_is_touched(2)

    if a_is_pressed:
        light_row(2)
        paint_box(right=1)
    if b_is_pressed:
        light_row(3)
        paint_box(left=3)
    if c_is_pressed:
        light_column(1)
        paint_box(bottom=1)
    if d_is_pressed:
        light_column(3)
        paint_box(top=3)


while True:
    four_buttons()

    microbit.sleep(10)

    # fade all pixels by one brightness level
    fade_display()
