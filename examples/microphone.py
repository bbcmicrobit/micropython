# Basic test for microphone.  This test should update the display when
# Button A is pressed and a loud or quiet sound *is* heard, printing the
# results. On Button B this test should update the display when a loud or
# quiet sound *was* heard, printing the results. On shake this should print
# the last sounds heard, you shoul try this test whilst making a loud sound 
# and a quiet one before you shake.

from microbit import *

icons = {
    SoundEvent.LOUD: Image.SQUARE,
    SoundEvent.QUIET: Image.SQUARE_SMALL
}

display.clear()
sound = microphone.current_event()

while True:
    if button_a.is_pressed():
        if microphone.current_event() == SoundEvent.LOUD:
            display.show(Image.SQUARE)
            uart.write('isLoud\n')
        elif microphone.current_event() == SoundEvent.QUIET:
            display.show(Image.SQUARE_SMALL)
            uart.write('isQuiet\n')
        sleep(500)
    display.clear()
    if button_b.is_pressed():
        if microphone.was_event(SoundEvent.LOUD):
            display.show(Image.SQUARE)
            uart.write('wasLoud\n')
        elif microphone.was_event(SoundEvent.QUIET):
            display.show(Image.SQUARE_SMALL)
            uart.write('wasQuiet\n')
        else:
            display.clear()
        sleep(500)
    display.clear()
    if accelerometer.was_gesture('shake'):
        sounds = microphone.get_events()
        soundLevel = microphone.sound_level()
        print(soundLevel)
        for sound in sounds:
            display.show(icons[sound])
            print(sound)
            sleep(500)