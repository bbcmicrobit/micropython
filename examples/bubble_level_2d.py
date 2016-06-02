"""
Two-dimensional bubble level which uses the accelerometer.
"""

from microbit import *

sensitivity = 'medium'  # Change to 'low', 'medium', or 'high' to adjust
divisors = {'low':64, 'medium':32, 'high':16}

def clamp(number, min, max):
    """Returns number limited to range specified by min and max, inclusive"""
    if number < min:
        return min
    elif number > max:
        return max
    else:
        return number

while True:
    x_grav, y_grav, _ = accelerometer.get_values()
    # Map raw values from accelerometer to pixels on display
    x_pixel = 4 - clamp(2 + x_grav // divisors.get(sensitivity), 0, 4)
    y_pixel = 4 - clamp(2 + y_grav // divisors.get(sensitivity), 0, 4)
    display.clear()
    display.set_pixel(x_pixel, y_pixel, 9)
    sleep(100)