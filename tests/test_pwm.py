#This tests that the duty cycle of PWM is (approximately) correct.

from microbit import *

def pins_connected(p0, p1):
    for i in 0,1,0,1,0,1,0,1:
        p0.write_digital(i)
        if p1.read_digital() != i:
            return False
        p1.write_digital(i)
        if p0.read_digital() != i:
            return False
    return True

def test_pwm(p0, p1):
    #Flip between modes many times
    for i in range(100):
        pin0.write_digital(i & 1)
        pin0.write_analog(i*10)
    #Now make sure it is still working.
    p0.write_digital(0)
    for val in 100, 200, 300, 500, 800, 900:
        print("Testing duty cycle", val)
        p0.write_analog(val)
        sleep(20)
        count = 0
        for i in range(10*1024):
            count += p1.read_digital()
        print("%+0.2f%%" % ((count-val*10)/val*10))
        # Allow +- 5%
        assert abs(count - val*10) < val//2

try:
    if pins_connected(pin0, pin1):
        test_pwm(pin0, pin1)
        print("PWM test: PASS")
        display.show(Image.HAPPY)
    else:
        print("Connect pin0 to pin1")
        display.show("?")
except Exception as ae:
    display.show(Image.SAD)
    raise

