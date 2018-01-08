# test speech module

import microbit
import speech

def test_funcs():
    print('Testing basic functions')
    ph = speech.translate('hello world')
    assert ph == ' /HEHLOW WERLD'
    speech.pronounce('PIHTHUN', pitch=32, speed=60, mouth=100, throat=150)
    speech.say('hello')
    speech.sing('YEHSTERDEY5')

def test_sleep():
    # check that sleep works ok with speech because they both use low-level timers
    # (this is really a test of the audio module)
    print('Testing sleep with speech')
    microbit.sleep(1)
    speech.say('hello world')
    microbit.sleep(1)

def test_timing():
    # test that speech takes the correct amount of time over many runs
    print('Testing timing of say function')
    for i in range(5):
        start = microbit.running_time()
        speech.say('hello world')
        microbit.sleep(1)
        stop = microbit.running_time()
        assert 800 < stop - start < 815

try:
    test_funcs()
    test_sleep()
    test_timing()
    print('Speech test: PASS')
except Exception as ae:
    raise
