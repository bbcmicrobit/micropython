
from microbit import display, Image

def eq(i, j):
    w = i.width()
    h = i.height()
    if w != j.width():
        return False
    if h != j.height():
        return False
    for x in range(w):
        for y in range(h):
            if i.get_pixel(x,y) != j.get_pixel(x,y):
                return False
    return True

TEST = Image("44444:45554:45654:45554:44444")

def test_blit():
    assert eq(TEST, TEST)
    i = Image(4, 4)
    i.blit(TEST, 1, 1, 5, 5)
    assert eq(i, Image("5554:5654:5554:4444"))
    i.fill(2)
    i.blit(TEST, -2, -2, 3, 3)
    assert eq(i, Image('0002:0002:0042:2222'))
    i.fill(2)
    i.blit(TEST, 2, 2, 3, 3)
    assert eq(i, Image('6542:5542:4442:2222'))
    i.fill(2)
    i.blit(TEST, 0, 0, 5, 5, -3, -3)
    assert eq(i, Image('5422:4422:2222:2222'))
    i.fill(2)
    i.blit(TEST, 2, 2, 2, 2, 1, 1)
    assert eq(i, Image('2222:2652:2552:2222'))
    i = TEST.copy()
    i.blit(i, 2, 2, 3, 3)
    assert eq(i, Image('65444:55454:44454:45554:44444'))
    i = TEST.copy()
    i.blit(i, -2, -2, 5, 5)
    assert eq(i, Image('00000:00000:00444:00455:00456'))
    i = TEST.copy()
    i.blit(i, 0, 0, 3, 3, 2, 2)
    assert eq(i, Image('44444:45554:45444:45455:44456'))
    i = Image(2, 7)
    i.fill(2)
    i.blit(TEST, -100, -100, 50, 50, 1, 1)
    assert eq(i, Image('22:20:20:20:20:20:20'))

def test_crop():
    assert eq(TEST.crop(-1, -1, 2, 2), Image('00:04'))
    assert eq(TEST.crop(1, 1, 2, 2), Image('55:56'))
    assert eq(TEST.crop(4, 4, 3, 3), Image('400:000:000'))

def test_shift():
    assert eq(TEST, TEST.shift_left(0))
    assert eq(TEST, TEST.shift_up(0))
    for n in range(-6, 7):
        assert eq(TEST.shift_left(n), TEST.shift_right(-n))
        assert eq(TEST.shift_up(n), TEST.shift_down(-n))
    assert eq(TEST.shift_left(1), Image('44440:55540:56540:55540:44440'))
    assert eq(TEST.shift_down(1), Image('00000:44444:45554:45654:45554'))

try:
    display.scroll("blit")
    test_blit()
    display.scroll("crop")
    test_crop()
    display.scroll("shift")
    test_shift()
    print("Image test: PASS")
    display.show(Image.HAPPY)
except Exception as ae:
    display.show(Image.SAD)
    raise
