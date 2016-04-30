#Plays a file on the specified pins.
import audio

def audio_generator(file):
    ln = -1
    frame = audio.AudioFrame()
    while ln:
        ln = file.readinto(frame)
        yield frame

def play_file(name, pin=None, return_pin=None):
    with open(name) as file:
        audio.play(audio_generator(file), pin=pin, return_pin=return_pin)
