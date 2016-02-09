### Flame simulation on the Microbit.
### Author: M. Schafer 2016
# This program has been placed into the public domain.

import microbit

# User adjustable values for range of brightness in flames.
min_brightness = 2
max_brightness = 8


#  fixed for the Microbit
display_width  = 5
display_height = 5

# mask to create fire shape. multiplies values %
Mask = [[ 88, 100, 100, 100, 88 ],
        [ 60,  95, 100,  95, 60 ], 
        [ 50,  88,  90,  88, 50 ], 
        [ 33,  75,  88,  75, 33 ],
        [ 10,  33,  66,  33, 10 ]  ]

# Generate a new bottom row of random values for the flames
def generate_line(line, start=min_brightness, end=max_brightness):
    "start and end define range of dimmest to brightest 'flames'"
    for i in range(display_width):
        line[i] = start + microbit.random(end-start)

# shift all values in the grid up one row
def shift_up(grid, newline):
    "Shift up lines in grid, add newline at bottom"
    for y in range(display_height-1,0,-1):
        for x in range(display_width):
            grid[y][x] = grid[y-1][x]
    # lowest line
    for x in range(display_width):
        grid[0][x] = newline[x]


# write a frame to the screen.
# Interpolate values based on percent
def interpolate_frame(screen, pcnt, grid, line):
    """ Interpolate new values by reading from grid and
         writing to the screen """
    # each row interpolates with the one before it
    for y in range(display_height-1,0,-1):
        for x in range(display_width):
            mask = Mask[y][x]
            newval = mask * ( (100-pcnt) * grid[y][x] + 
                              pcnt * grid[y-1][x]      ) / 10000.0
            screen.set_pixel(x, y, int(newval))
    # first row interpolates with the "next" line
    for x in range(display_width):
        mask = Mask[y][x]
        newval = mask * ( (100-pcnt) * grid[0][x] + 
                          pcnt * line[x]             ) / 10000.0
        screen.set_pixel(x, 0, int(newval))

## Setup

### These are simple arrays of pixels.
### We can set them up manually or by using list comprehensions.
#line = [0,0,0,0,0]
line = [0 for i in range(display_width)]

#grid = [[0,0,0,0,0],
#        [0,0,0,0,0],
#        [0,0,0,0,0],
#        [0,0,0,0,0],
#        [0,0,0,0,0] ]
grid = [[0 for i in range(display_width)] for i in range(display_height)]

screen = microbit.display
percent = 0     # counter to see when to re-interpolate
debounce = 0    # do not detect the buttons too often (>100ms)
sleeptime = 0   # delay between updates
percent_increment = 25  # how fast we interpolate fire


# loop forever
while True:
    if percent > 100:
        # move everything up a line, insert new bottom row
        generate_line(line)
        shift_up(grid, line)
        percent = 0

    # Check Buttons to see if changing
    # button_a = smoothness
    if microbit.button_a.is_pressed() and debounce > 100:
        percent_increment += 5
        debounce = 0
        if percent_increment > 50:
             percent_increment = 1
        print("percent interpolte=",percent_increment)
    # button_b = delay
    if microbit.button_b.is_pressed() and debounce > 100:
        sleeptime += 10
        debounce = 0
        if sleeptime > 100:
             sleeptime = 0
        print("sleeptime=",sleeptime)
    # draw frame and sleep
    interpolate_frame(screen, percent, grid, line)
    microbit.sleep(sleeptime)
    # update main counters
    percent += percent_increment
    debounce += max(sleeptime, 10)

