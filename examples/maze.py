"""
A simple maze program.  You are the flashing dot and can walk around
using the accelerometer.
"""

import microbit

d = microbit.display
ac = microbit.accelerometer

# the maze data, as binary numbers (outside walls are added automatically)
maze = [
    0b0000000000000000,
    0b0100010101011110,
    0b0100010101010010,
    0b0111110100000000,
    0b0000000111111110,
    0b0111111101000000,
    0b0101010001011100,
    0b0101000100000100,
    0b0100011111111100,
    0b0101010001000110,
    0b0101000100010010,
    0b0101010111010110,
    0b0111010101010010,
    0b0000010100010010,
    0b0111110111111110,
    0b0000000000000000,
]

def get_maze(x, y):
    if 0 <= x < 16 and 0 <= y < 16:
        return (maze[y] >> (15 - x)) & 1
    else:
        return 1

def draw(x, y, tick):
    img = microbit.Image(5,5)
    for j in range(5):
        for i in range(5):
            img.set_pixel(i, j, get_maze(x + i - 2, y + j - 2)*5)

    # draw the player, flashing
    img.set_pixel(2, 2, (tick & 1)*4+5)
    d.show(img)

def main():
    x = 0
    y = 0
    tick = 0
    while True:
        tick += 1
        if tick == 4:
            # walk around, with collision detection
            tick = 0
            if ac.get_x() > 200 and get_maze(x + 1, y) == 0:
                x += 1
            elif ac.get_x() < -200 and get_maze(x - 1, y) == 0:
                x -= 1
            elif ac.get_y() > 200 and get_maze(x, y + 1) == 0:
                y += 1
            elif ac.get_y() < -200 and get_maze(x, y - 1) == 0:
                y -= 1
            x = min(15, max(0, x))
            y = min(15, max(0, y))

        # draw the maze
        draw(x, y, tick)


        microbit.sleep(50)

main()
