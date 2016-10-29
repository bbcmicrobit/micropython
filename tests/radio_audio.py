
import audio
import radio
from microbit import button_a, button_b, display, running_time, sleep
import os

def sample_generator(filename):
    buf = audio.AudioFrame()
    with open(filename, "rb") as file:
        ln = -1
        while ln:
            ln = file.readinto(buf)
            yield buf

# 1 second of 128Hz sawtooth wave.
def sawtooth_generator():
    sawtooth = audio.AudioFrame()
    for i in range(32):
        sawtooth[i] = i*8+4
    for i in range(256):
        yield sawtooth


def send():
    display.clear()
    radio.on()
    radio.config(channel=90, power=4)
    if "sample.raw" in os.listdir():
        gen = sample_generator("sample.raw")
    else:
        gen = sawtooth_generator()
    start = running_time()
    sent = 0
    for f in gen:
        # One frame every 4ms = 8kHz
        while sent > ((running_time() - start) >> 2) + 3:
            sleep(1)
        radio.send_bytes(f)
        sent += 1
    print(sent)

def play():
    display.clear()
    radio.on()
    radio.config(channel=90, queue=12)
    count = -1
    def gen():
        recvd = audio.AudioFrame()
        empty = audio.AudioFrame()
        while True:
            if radio.receive_bytes_into(recvd) == 32:
                yield recvd
            else:
                yield empty
            if button_a.is_pressed() and button_b.is_pressed():
                return
    audio.play(gen())

while True:
    message = "Press button a to send 'sample.raw' or sawtooth wave. Press button b to play received waveform. Press both buttons to stop."
    display.scroll(message, delay=100, wait=False)
    message_end = running_time() + len(message)*600
    if button_a.is_pressed() and button_b.is_pressed():
        break
    while True:
        sleep(50)
        if button_a.is_pressed():
            send()
            break
        if button_b.is_pressed():
            play()
            break
        if running_time() > message_end:
            break
