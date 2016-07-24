#This tests that the mode of pins can be configured properly and that pull works OK.

from microbit import *
import music

BIG_PINS = pin0, pin1, pin2
DISPLAY_PINS = pin3, pin4, pin6, pin7, pin9, pin10

def test_mode_switching():
    print ("Switching mode from analog to digital on big pins")
    for p0 in BIG_PINS:
        p0.write_analog(100)
        for p1 in BIG_PINS:
             p1.write_digital(0)
        p0.write_analog(0)
    for p in BIG_PINS:
        p.write_analog(100)
        assert p.get_mode() == "write_analog"
        p.read_digital()
        assert p.get_mode() == "read_digital"
        p.write_analog(0)
        assert p.get_mode() == "unused"
        p.write_digital(1)
        assert p.get_mode() == "write_digital"
        p.read_analog()
        assert p.get_mode() == "unused"

def test_display_pins():
    print ("Switching mode to digital for display pins")
    for p0 in DISPLAY_PINS:
        try:
            p0.write_digital(0)
        except ValueError:
            pass
        try:
            p0.write_analog(0)
        except ValueError:
            pass
    display.off()
    for p0 in DISPLAY_PINS:
        p0.write_digital(0)
    display.on()
    display.off()
    for p0 in DISPLAY_PINS:
        p0.write_analog(200)
    display.on()

def test_music():
    print ("Switching mode to digital for music pins")
    music.play(music.DADADADUM,pin=pin0,wait=False)
    try:
        pin0.write_digital(0)
    except ValueError:
        pass
    music.stop()
    pin0.write_digital(0)
    
def test_pull():
    print ("Setting pull on big pins")
    for p in BIG_PINS:
        p.read_digital()
        p.set_pull(p.PULL_UP)
        assert p.get_pull() == p.PULL_UP
        assert p.read_digital()
        p.set_pull(p.PULL_DOWN)
        assert p.get_pull() == p.PULL_DOWN
        assert p.read_digital() == 0
        p.set_pull(p.NO_PULL)
        assert p.get_pull() == p.NO_PULL
    print ("Setting pull on display pins")
    display.off()
    for p in DISPLAY_PINS:
        p.read_digital()
        p.set_pull(p.PULL_UP)
        assert p.read_digital()
        p.set_pull(p.PULL_DOWN)
        assert p.read_digital() == 0
    display.on()

try:
    test_mode_switching()
    test_display_pins()
    test_music()
    test_pull()
    print("Pin test: PASS")
    display.show(Image.HAPPY)
except Exception as ae:
    display.show(Image.SAD)
    raise

