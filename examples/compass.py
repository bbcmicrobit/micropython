"""
    compass.py
    ~~~~~~~~~~

    Creates a compass.

    To calibrate the compass, lay it flat and spin it horizontally. For best
    results during calibration, try not to tilt the compass up and down.
"""
from microbit import *

needles = Image.CLOCKFACE

# You need to spin the microbit about a few times to help calibrate the compass

# Scroll a message to encourage user to help calibrate the compass
display.scroll("Spin Me!")

# Start calibrating
compass.calibrate()

# Spin needle until calibrated.
while compass.is_calibrating():
    for i in range(12):
        display.show(needles[i])
        sleep(80)
display.clear()

# Try to keep needle pointed in (roughly) the same direction
while True:
    sleep(500)
    needle = ((15 - compass.heading()) // 30) % 12
    display.show(needles[needle])
