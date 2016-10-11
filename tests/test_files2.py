
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

def clear_files():
    for f in os.listdir():
        os.remove(f)

def test_interleaved_small_files():
    for i in range(80):
        name = "%d.dat" % i
        write_data_to_file(name, i*3, 16, 6)
    for i in range(0, 80, 2):
        os.remove("%d.dat" % i)
    for i in range(80, 120):
        name = "%d.dat" % i
        write_data_to_file(name, i*3, 16, 6)
        verify_file(name, data_stream(i*3), 16, 6, 'b')
    for i in range(1, 80, 2):
        os.remove("%d.dat" % i)
    for i in range(80, 120):
        os.remove("%d.dat" % i)
    assert not os.listdir()

def test_interleaved_large_files():
    out_buf = bytearray(100)
    for i in range(100):
        out_buf[i] = 100-i
    with open("test1.dat", "wb") as fd1:
        with open("test2.dat", "wb") as fd2:
            for i in range(60):
                fd1.write(out_buf)
                fd2.write(out_buf)
    os.remove("test2.dat")
    with open("test3.dat", "wb") as fd3:
        for i in range(60):
            fd3.write(out_buf)
    assert sorted(os.listdir()) == [ "test1.dat", "test3.dat" ]
    in_buf = bytearray(100)
    with open("test1.dat", "rb") as fd:
        for i in range(60):
            fd.readinto(in_buf)
            assert in_buf == out_buf
    with open("test3.dat", "rb") as fd:
        for i in range(60):
            fd.readinto(in_buf)
            assert in_buf == out_buf

display.clear()
try:
    clear_files()
    test_interleaved_small_files()
    test_interleaved_large_files()
    print("File test: PASS")
    display.show(Image.HAPPY)
except Exception as ae:
    display.show(Image.SAD)
    raise
