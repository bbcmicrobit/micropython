from microbit import display, sleep, button_a
import audio
import math

def repeated_frame(frame, count):
    for i in range(count):
        yield frame

# Press button A to skip to next wave.
def show_wave(name, frame, duration=1500):
    display.scroll(name + " wave", wait=False,delay=100)
    audio.play(repeated_frame(frame, duration),wait=False)
    for i in range(75):
        sleep(100)
        if button_a.is_pressed():
            display.clear()
            audio.stop()
            break

frame = audio.AudioFrame()

for i in range(len(frame)):
    frame[i] = int(math.sin(math.pi*i/16)*124+128.5)
show_wave("Sine", frame)

triangle = audio.AudioFrame()

QUARTER = len(triangle)//4
for i in range(QUARTER):
    triangle[i] = i*15
    triangle[i+QUARTER] = 248-i*15
    triangle[i+QUARTER*2] = 128-i*15
    triangle[i+QUARTER*3] = i*15+8
show_wave("Triangle", triangle)

square = audio.AudioFrame()

HALF = len(square)//2
for i in range(HALF):
    square[i] = 8
    square[i+HALF] = 248
show_wave("Square", square)
sleep(1000)

for i in range(len(frame)):
    frame[i] = 252-i*8
show_wave("Sawtooth", frame)

del frame

#Generate a waveform that goes from triangle to square wave, reasonably smoothly.
frames = [ None ] * 32
for i in range(32):
    frames[i] = frame = audio.AudioFrame()
    for j in range(len(triangle)):
        frame[j] = (triangle[j]*(32-i) + square[j]*i)>>5

def repeated_frames(frames, count):
    for frame in frames:
        for i in range(count):
            yield frame


display.scroll("Ascending wave", wait=False)
audio.play(repeated_frames(frames, 60))
