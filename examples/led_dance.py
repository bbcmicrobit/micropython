# Light LEDs at random and make them fade over time
#
# Usage:
#
#    led_dance(delay)
#
# 'delay' is the time between each new LED being turned on.
#
# TODO The random number generator is not great. Perhaps the accelerometer
# or compass could be used to add entropy.

import microbit

def led_dance(delay):
    dots = [ [0]*5, [0]*5, [0]*5, [0]*5, [0]*5 ]
    microbit.display.set_display_mode(1)
    while True:
        dots[microbit.random(5)][microbit.random(5)] = 128
        for i in range(5):
            for j in range(5):
                microbit.display.image.set_pixel(i, j, dots[i][j])
                dots[i][j] = int(dots[i][j]/2)
        microbit.sleep(delay)

led_dance(100)
