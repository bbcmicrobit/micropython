from microbit import *

def countDown(x):
    while x > 0:
        msg = "%d" % (x)
        display.print(msg)
        sleep(500)
        x = x - 1
    display.clear()


def spin(direction):
    if direction is "b":
        display.print(Image.ARROW_N)
        sleep(100)
        display.print(Image.ARROW_NE)
        sleep(100)
        display.print(Image.ARROW_E)
        sleep(100)
        display.print(Image.ARROW_SE)
        sleep(100)
        display.print(Image.ARROW_S)
        sleep(100)
        display.print(Image.ARROW_SW)
        sleep(100)
        display.print(Image.ARROW_W)
        sleep(100)
        display.print(Image.ARROW_NW)
        sleep(100)
        display.print(Image.ARROW_N)
        sleep(100)
        display.print(Image.ARROW_NE)
        sleep(100)
        display.print(Image.ARROW_E)
        sleep(100)
        flash("b")

    if direction is "a":
        display.print(Image.ARROW_N)
        sleep(100)
        display.print(Image.ARROW_NW)
        sleep(100)
        display.print(Image.ARROW_W)
        sleep(100)
        display.print(Image.ARROW_SW)
        sleep(100)
        display.print(Image.ARROW_S)
        sleep(100)
        display.print(Image.ARROW_SE)
        sleep(100)
        display.print(Image.ARROW_E)
        sleep(100)
        display.print(Image.ARROW_NE)
        sleep(100)
        display.print(Image.ARROW_N)
        sleep(100)
        display.print(Image.ARROW_NW)
        sleep(100)
        display.print(Image.ARROW_W)
        sleep(100)
        flash("a")

def flash(direction):
    if direction is "b":
        a = 10
        while a > 0:
            display.print(Image.ARROW_E)
            sleep(200)
            display.clear()
            sleep(200)
            a = a - 1
    if direction is "a":
        a = 10
        while a > 0:
            display.print(Image.ARROW_W)
            sleep(200)
            display.clear()
            sleep(200)
            a = a - 1



countDown(5)
howLong = random(7000)
sleep(howLong)

while (not button_a.is_pressed()) and (not button_b.is_pressed()):
    display.print(Image.HAPPY)
    sleep(1)

if button_a.is_pressed():
    spin("a")
    sleep(1000)
else:
    spin("b")
    sleep(1000)

display.clear()
