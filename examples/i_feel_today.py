"""
Program that shows different emotions.
Push button "A" to become sadder and "B" to become happier.
"""

from microbit import *

horror = Image("09090:00000:09990:90009:99999")
better_meh = Image("00000:09090:00000:99999:00000")
joy = Image("09090:00000:99999:90009:09990")

emotions = [horror, Image.SAD, better_meh, Image.HAPPY, joy]
current_emotion = 2

while True:
    if button_a.get_presses():
        current_emotion = max(current_emotion - 1, 0)
    elif button_b.get_presses():
        current_emotion = min(current_emotion + 1, 4)
    display.show(emotions[current_emotion])