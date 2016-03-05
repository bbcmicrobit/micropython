from microbit import *
import random

clockwise_images = [
    Image.ARROW_W,
    Image.ARROW_NW,
    Image.ARROW_N,
    Image.ARROW_NE,
    Image.ARROW_E,
    Image.ARROW_SE,
    Image.ARROW_S,
    Image.ARROW_SW,
    Image.ARROW_W,
    Image.ARROW_NW,
    Image.ARROW_N,
    Image.ARROW_NE,
    Image.ARROW_E,
]

anti_clockwise_images = reversed(clockwise_images)


def countDown(x):
    while x > 0:
        msg = "%d" % (x)
        display.show(msg)
        sleep(500)
        x = x - 1
    display.clear()


def spin(direction):
    if direction is "b":
        display.show(clockwise_images, 100)
    if direction is "a":
        display.show(anti_clockwise_images, 100)
    flash(direction)


def flash(direction):
    number_of_flashes = 10
    direction_image = Image.ARROW_N
    if direction is "b":
        direction_image = Image.ARROW_E
    if direction is "a":
        direction_image = Image.ARROW_W
    while number_of_flashes > 0:
        display.show(direction_image)
        sleep(200)
        display.clear()
        sleep(200)
        number_of_flashes = number_of_flashes - 1


countDown(5)
howLong = random.randrange(7000)
display.show(Image.SAD)
sleep(howLong)

while (not button_a.is_pressed()) and (not button_b.is_pressed()):
    display.show(Image.HAPPY)
    sleep(1)

if button_a.is_pressed():
    spin("a")
else:
    spin("b")

sleep(1000)
display.clear()
