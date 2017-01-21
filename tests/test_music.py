
from microbit import display, Image, pin0, pin1, pin2, pin8
import music

PINS = pin0, pin1, pin2, pin8
FREQS = 400, 500, 600, 700, 800, 900

def test_rapid_pin_switch():
    for i in range(20):
        for pin in PINS:
            music.play(music.NYAN, pin, wait=False)

def test_rapid_pitch_switch():
    for i in range(20):
        for freq in FREQS:
            music.pitch(freq, wait=False)

def test_repeated_stop():
    for i in range(20):
        music.stop()

def test_repeated_reset():
    for i in range(20):
        music.reset()

def test_repeated_set_tempo():
    for i in range(20):
        music.set_tempo(ticks=i%4+1, bpm=i+100)

def test_all_pins_free():
    for pin in PINS:
        pin.read_digital()

display.clear()
try:
    test_rapid_pin_switch()
    test_rapid_pitch_switch()
    test_repeated_stop()
    test_repeated_reset()
    test_repeated_set_tempo()
    test_all_pins_free()
    print("File test: PASS")
    display.show(Image.HAPPY)
except Exception as ae:
    display.show(Image.SAD)
    raise
