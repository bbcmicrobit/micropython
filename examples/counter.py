#A counter for the microbit using the a button.

import microbit
ctr = 0  # Initialize counter
while True: # Loop forever
  if microbit.button_a.is_pressed():   
    microbit.display.scroll(str(ctr)) #Display counter
    ctr = ctr + 1  #Increment counter by one
  microbit.sleep(100)
