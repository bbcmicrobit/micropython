######################################################
# A watch (as in a small clock for your wrist or pocket)
# 
# Button A sets the mode: Clock or Setting time
# Button B 
#     in clock mode: shows the time as a scrolling display 
#     in setting mode: increments the time
# 
# The LED array displays the clock time in the format hh:mm. 
# The digits of the time are represented by columns of LEDs.
# 
# The digits 1 - 5 are represented by more LEDs being lit from 
# the bottom up.
# 
# For instance the digit 3  would look like:
# 
#    .
#    .
#    X
#    X
#    X
# 
# 
# The digits 6 - 9 are represented by LEDs being turned off from
# the bottom up. The digit 6 would look like:
# 
#    X
#    X
#    X
#    X
#    .
# 
# The centre column is a colon flashing once a second to separate hours from minutes.
# 
# The time 17:49 would look like:
# 
#    . X . . X
#    . X . X .
#    . X . X .
#    . . . X .
#    X . . X .
# 
# 
######################################################

from microbit import *

# Tweak CLOCK_ADJUST to make your system clock more accurate.
# My clock is too fast by 4 seconds every minute so I use 4/60.
# If your clock is too slow by 3 seconds every minute use -3/60.
CLOCK_ADJUST = 4/60 

last_button_a_state = False
last_button_b_state = False
last_display_time = 0
base_time = 0
mode = 0

modes = {0:"clock", 1:"set h", 2:"mx10", 3:"m"}


def decode_time(milliseconds):
    """Converts a time in milliseconds into a string with hours:minutes,"""
    mins = int(milliseconds / (1000 * 60) % 60)
    hrs = int(milliseconds / (1000 * 60 * 60) % 24)
    return "{h:0>2}:{m:0>2}".format(h=hrs, m=mins)


def show_time(time):
    time_string = decode_time(time)

    for i in range(5):
        if time_string[i].isdigit():
            d  = int(time_string[i])
            plot_LED_column(i, d)

    show_colon(mode==0 and int((time / 1000) % 2))


def show_colon(visible):
    display.set_pixel(2, 1, visible*9)
    display.set_pixel(2, 3, visible*9)


def get_clock_time():
    global base_time
    sys_time = running_time() / (1 + CLOCK_ADJUST)
    time = (sys_time - base_time) % (24 * 60 * 60 * 1000)
    base_time = sys_time - time 
    return time


def plot_LED_column(column, number):
    """plots a column of LEDs to represent a number from 0 - 9"""
    if number > 9:
        number = 9

    if number <= 5:
        for i in range(4, -1, -1):
            if i < 5 - number:
                display.set_pixel(column, i, 0)
            else:
                display.set_pixel(column, i, 9)

    if number > 5:
        for i in range(4, -1, -1):
            if i < 5 - (number - 5):
                display.set_pixel(column, i, 9)
            else:
                display.set_pixel(column, i, 0)


while True:
    # detect a change in button A's state, the Mode button
    button_a_state = button_a.is_pressed()
    if button_a_state != last_button_a_state:
        last_button_a_state = button_a_state

        #increment the mode
        if button_a_state == True:
            mode = (mode + 1) % 4
            display.scroll(modes[mode])
            show_time(get_clock_time())

    # detect a change in button B's state, the increment / select button
    button_b_state = button_b.is_pressed()
    if button_b_state != last_button_b_state:
        last_button_b_state = button_b_state

        if button_b_state == True:
            # button B's action depends on the current mode
            if mode == 0:   #show time
                display.scroll(decode_time(get_clock_time()))
            elif mode == 1: #setting time: increment hour units
                base_time = base_time - (60 * 60 * 1000)
            elif mode == 2: #setting time: increment minute tens
                base_time = base_time - (10 * 60 * 1000)
            elif mode == 3: #setting time: increment minute units
                base_time = base_time - (60 * 1000)

            show_time(get_clock_time())
  

    #If in clock mode update the display every second
    if mode == 0:
        display_time = running_time() - last_display_time
        if display_time >= 1000:
            last_display_time = display_time
            show_time(get_clock_time())

    sleep(100)
