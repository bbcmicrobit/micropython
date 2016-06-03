"""
    die20.py
    ~~~~~~~~
    Rolls a 20-sided die. If a 20 is rolled, display a short series of images and
    play some music. The skull image represents 20. If a 1 is rolled, display a sad
    face and play some music.
    The music isn't required, but if you want to hear it, you'll need a
    speaker/buzzer/headphones connected to P0 and GND.
"""

from microbit import *
import random
import music

# A list of images to display if 20 (crit) is rolled
critimages = [Image.HEART, Image.HEART_SMALL, Image.SKULL]

# Loop forever
while True:
    # User can press button A or shake the microbit to "roll"
    if button_a.was_pressed() or accelerometer.was_gesture("shake"):
        dice = random.randint(1, 20)
        if dice == 20:
            # Set wait to False to that it's not blocking.
            # Music will play while images are cycled through.
            music.play(music.POWER_UP, wait=False)
            display.show(critimages, delay=1400)
        elif dice == 1:
            music.play(music.WAWAWAWAA, wait=False)
            display.show(Image.SAD)
        # For single digit numbers, show the number. Scroll for double-digits.
        elif 10 > dice > 1:
            display.show(str(dice))
        else:
            display.scroll(str(dice), wait=False, loop=True)
