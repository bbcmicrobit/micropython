from microbit import *
import power
import log

# Log the temperature every 5 minutes
@run_every(min=5)
def log_temperature():
    log.add(temp=temperature())

while True:
    # Display the temperature when button A is pressed
    if button_a.is_pressed():
        display.scroll(temperature())
    # To go sleep, wake up when button A is pressed, and ensure the
    # function scheduled with run_every still executes in the background
    power.deep_sleep(wake_on=button_a, run_every=True)
