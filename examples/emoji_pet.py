"""
    emoji_pet.py
    ~~~~~~~~

    A reactive "pet" which changes face depending on certain inputs
    editable by the user.
    Emotions are represented by animations that use the display.show function.
    You can optionally get reactive sounds from by
    connecting a speaker/buzzer/headphone to P0 and GND.
"""
from microbit import *
import music

# Various emoji states of the face
neutral_face = Image("09090:00000:99999:00000:00000")
neutral_face2 = Image("09090:00000:00000:99999:00000")

squiggle_face1 = Image("09090:00000:90000:99999:00009")
squiggle_face2 = Image("09090:00000:00009:99999:90000")

grin_face = Image("09090:00000:90009:09990:00000")
happy_face = Image("09090:00000:99999:90009:09990")

frowning_face = Image("09090:00000:09990:90009:00000")
sad_face = Image("09090:00000:09990:90009:90009")

small_tongue = Image("07090:00000:99999:00990:00000")


def get_happy(delay=500):
    # A series of happy faces
    music.play(music.BA_DING, wait=False)
    display.show([neutral_face, grin_face, happy_face], delay=delay)


def get_sad(delay=500):
    # A series of sad faces
    music.play(music.POWER_DOWN, wait=False)
    display.show([neutral_face, frowning_face, sad_face], delay=delay)


def get_neutral(delay=500):
    # A series of grim / neutral faces
    display.show([
        neutral_face2,
        squiggle_face1,
        squiggle_face2,
        neutral_face
    ], delay=delay)


def get_silly(delay=500):
    # A series of silly faces
    music.play(music.PYTHON, wait=False)
    display.show([neutral_face, small_tongue], delay=delay)


while True:
    display.show(neutral_face)
    if button_a.is_pressed() or pin1.is_touched():
        get_happy()
    if button_b.is_pressed() or pin2.is_touched():
        get_silly()
    if accelerometer.was_gesture("shake"):
        get_sad()
    else:
        display.show(neutral_face)
