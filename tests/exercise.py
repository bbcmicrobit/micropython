# Exercises the micro:bit - NOT EXHAUSTIVE!
from microbit import *
import music
import random


# Press A to start.
while True:
    if button_a.was_pressed():
        break
    else:
        display.show(Image.ARROW_W)
        sleep(200)
        display.clear()
        sleep(200)


# Asyncronously play a jolly little tune (connect speaker to pin0 and GND)
music.play(music.NYAN, wait=False)


# Grab all the built in images.
images = [getattr(Image, img) for img in dir(Image)
          if type(getattr(Image, img)) == Image]
# ... and cycle through them on the display.
pause = 1000
for img in images:
    display.show(img)
    sleep(pause)
    pause -= 50
    if pause < 100:
        pause = 100
display.clear()


# Aural testing of the accelerometer.
display.scroll("Accelerometer")
display.show("X")
while not button_a.is_pressed():
    music.pitch(abs(accelerometer.get_x()), 20)
sleep(500)
display.show("Y")
while not button_a.is_pressed():
    music.pitch(abs(accelerometer.get_y()), 20)
sleep(500)
display.show("Z")
while not button_a.is_pressed():
    music.pitch(abs(accelerometer.get_z()), 20)


# Aural testing of the compass.
display.scroll("Compass")
compass.calibrate()
while not button_b.is_pressed():
    music.pitch(abs(compass.heading()), 20)


# Pixel brightness.
display.scroll("Display")
dots = [ [0]*5, [0]*5, [0]*5, [0]*5, [0]*5 ]
while not button_a.is_pressed():
    dots[random.randrange(5)][random.randrange(5)] = 9
    for i in range(5):
        for j in range(5):
            display.set_pixel(i, j, dots[i][j])
            dots[i][j] = max(dots[i][j] - 1, 0)
    sleep(50)


# ??? Add further tests here...


# Finished!
display.scroll("Finished!")
display.show(Image.HAPPY)
