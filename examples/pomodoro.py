"""
A simple pomodoro timer.
It times a 25 minute work session then 5 minutes rest.
Press the reset button to restart the timer.
"""

from microbit import *

# Tweak CLOCK_ADJUST to make your system clock more accurate.
# My clock is too fast by 4 seconds every minute so I use 4/60.
# If your clock is too slow by 3 seconds every minute use -3/60.

CLOCK_ADJUST = 4/60 

ALL_LEDS_ON = Image('99999:'*5)


def index_to_xy(i):
    x = i % 5
    y = int(i / 5)
    return x, y    


def show_alarm():
    for i in range(10):
        display.show(ALL_LEDS_ON)
        sleep(250)
        display.clear()
        sleep(250)


def run_timer(seconds, LED_state):
    interval = int(seconds * 1000 / 25 * (1 + CLOCK_ADJUST))
    intervals_remaining = 25
    timer = running_time()

    while intervals_remaining > 0:

        # Every interval set a pixel to LED_state
        time = running_time()
        if time - timer >= interval:
            timer = time
            x, y = index_to_xy(intervals_remaining - 1)
            display.set_pixel(x, y, LED_state)
            intervals_remaining -= 1


print("pomodoro timer")


# time the pomodoro work session, 25 minutes
display.scroll("Go!")
display.show(ALL_LEDS_ON)
run_timer(25 * 60, 0)
show_alarm()


# time the pomodoro break, 5 minutes
display.scroll("break")
display.clear()
run_timer(5 * 60, 1)
show_alarm()

display.show(Image.NO)

print("finished\n")
print("Press the reset button to restart timer.")
