from microbit import *
import log

# Configure the labels and select a time unit for the timestamp
log.set_labels('temp', 'brightness', timestamp=log.SECONDS)

# Send each data row to the serial output
log.set_mirroring(True)

continue_logging = True

# This decorator schedules this function to run every 10s 50ms
@run_every(s=10, ms=50)
def log_data():
    """Log the temperature and light level, and display an icon."""
    global continue_logging
    if continue_logging:
        display.show(Image.SURPRISED)
        try:
            log.add(temp=temperature(), brightness=display.read_light_level())
        except OSError:
            continue_logging = False
            display.scroll("Log full")
        sleep(500)

while True:
    if button_a.is_pressed() and button_b.is_pressed():
        display.show(Image.CONFUSED)
        # Delete the log file using the "full" options, which takes
        # longer but ensures the data is wiped from the device
        log.delete(full=True)
        continue_logging = True
    elif button_a.is_pressed():
        display.show(Image.HAPPY)
        # Log only the light level, the temp entry will be empty. If the log
        # is full this will throw an exception and the programme will stop
        log.add({
            "brightness": display.read_light_level()
        })
    else:
        display.show(Image.ASLEEP)
    sleep(500)
