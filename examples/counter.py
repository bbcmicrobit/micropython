"""
    counter.py
    ~~~~~~~~~~
    Creates a counter that increments on pressing button a.
    Scrolls the current count on the led display.
"""

import microbit
ctr = 1                               # Initialize counter
while True:                           # Loop forever
  ctr = ctr + microbit.button_a.get_presses()   
  microbit.button_a.reset_presses()   #Get the amount of presses and add it to ctr
  microbit.display.scroll(str(ctr))   #Display the counter
  microbit.sleep(100)                 #Sleep for 100ms
