
import os
from microbit import display, Image


def data_stream(seed):
    val = seed&0xffff
    A = 33
    C = 509
    while True:
        val = (A*val+C)&0xffff
        yield val >> 8

def text_stream(seed):
    val = seed&0xffff
    A = 33
    C = 509
    while True:
        val = (A*val+C)&0xffff
        yield chr(val >> 10 + 34)

def write_data_to_file(name, seed, n, count):
    print ("Writing data to " + name)
    buf = bytearray(n)
    d = data_stream(seed)
    with open(name, 'wb') as fd:
        for i in range(count):
            for j in range(n):
                buf[j] = next(d)
            fd.write(buf)

def verify_file(name, stream, n, count, mode):
    print ("Verifying data in " + name)
    byte_count = 0
    with open(name, mode) as fd:
        while True:
            buf = fd.read(n)
            assert len(buf) <= n
            if not buf:
                break
            for b in buf:
                expected = next(stream)
                assert b == expected, "expected %d, got %d at %d" % (expected, b, byte_count)
                byte_count += 1
    assert byte_count == n*count, "expected count of %d, got %d" % (n*count, byte_count)

def write_text_to_file(name, seed, n, count):
    print ("Writing text to " + name)
    buf = bytearray(n)
    d = text_stream(seed)
    with open(name, 'w') as fd:
        for i in range(count):
            s = ''
            for j in range(n):
                s += next(d)
            fd.write(s)

def clear_files():
    for f in os.listdir():
        os.remove(f)

def test_small_files():
    name = "test1.dat"
    write_data_to_file(name, 12, 16, 5)
    verify_file(name, data_stream(12), 20, 4, 'b')
    name = "test2.dat"
    write_data_to_file(name, 101, 64, 5)
    verify_file(name, data_stream(101), 32, 10, 'b')
    assert os.listdir() == [ "test1.dat", "test2.dat" ]
    os.remove("test1.dat")
    os.remove("test2.dat")
    assert not os.listdir()

def test_text_file():
    name = "test1.txt"
    write_text_to_file(name, 13, 16, 2)
    verify_file(name, text_stream(13), 16, 2, 't')
    name = "test2.txt"
    write_text_to_file(name, 35, 16, 5)
    verify_file(name, text_stream(35), 20, 4, 't')
    assert os.listdir() == [ "test1.txt", "test2.txt" ]
    os.remove("test1.txt")
    os.remove("test2.txt")
    assert not os.listdir()

def test_many_files():
    for i in range(80):
        name = "%d.dat" % i
        write_data_to_file(name, i*3, 16, 4)
        verify_file(name, data_stream(i*3), 16, 4, 'b')
    for i in range(80):
        os.remove("%d.dat" % i)
        name = "_%d.dat" % i
        write_data_to_file(name, i*3, 16, 4)
        verify_file(name, data_stream(i*3), 16, 4, 'b')
    for i in range(80):
        os.remove("_%d.dat" % i)
    assert not os.listdir()

display.clear()
try:
    clear_files()
    test_small_files()
    test_many_files()
    test_text_file()
    print("File test: PASS")
    display.show(Image.HAPPY)
except Exception as ae:
    display.show(Image.SAD)
    raise
