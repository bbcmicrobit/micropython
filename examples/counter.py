"""
    counter.py
    ~~~~~~~~~~
    Creates a counter that increments on pressing button a.
    Scrolls the current count on the led display.
"""

import microbit
ctr = 0                                         # Initialize counter
while True:                                     # Loop forever
    ctr = ctr + microbit.button_a.get_presses() # Count the amount of presses 
    if ctr > 0:                                 # Only execute if not zero
        microbit.display.scroll(str(ctr))       # Display the counter
    microbit.sleep(100)                         # Sleep for 100ms
