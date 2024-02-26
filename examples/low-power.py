"""
Shows a silly face on the display every 10 seconds.
When button A is pressed it goes into Deep Sleep mode, and wakes 30 minutes later,
or by pressing button A again.
When button B is pressed it goes into to Off mode.
"""
from microbit import *
import power

@run_every(s=10)
def silly_face():
    display.show(Image.SILLY)
    sleep(400)

while True:
    if button_b.is_pressed():
        display.scroll("Off")
        # In this mode the micro:bit can only wake up via the reset button
        power.off()
        # This line of code will never be executed, as waking up from this
        # mode starts the programme from the beginning
        display.show(Image.SURPRISED)
    elif button_a.is_pressed():
        display.show(Image.ASLEEP)
        sleep(300)
        # Go into Deep Sleep with multiple wake up sources
        power.deep_sleep(
            wake_on=(pin0, pin1, button_a),
            ms=30*1000,        # In 30 seconds it wakes up anyway
            run_every=False,   # Stops run_every from waking up the board
        )
        # When the micro:bit wakes up will it continue running from here
        # Blink a few times to show you are waking up
        display.show([Image("99099:09090:99099:09990"), Image.ASLEEP] * 3, 250)
    display.show(Image.HAPPY)
    sleep(200)
