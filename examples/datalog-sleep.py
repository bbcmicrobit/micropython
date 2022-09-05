from microbit import *
import power
import log

# Log the temperature every 5 minutes
@run_every(s=5)
def log_temperature():
    log.add(temp=temperature())

while True:
    if button_a.is_pressed():
        # Display the temperature when button A is pressed
        display.scroll(temperature())
    # To go sleep and wake up with run_every or button A
    power.deep_sleep(wake_on=button_a, run_every=True)
