#Plays a file on the specified pins.
import audio

def audio_generator(file, frame):
    ln = -1
    while ln:
        ln = file.readinto(frame)
        yield frame

def play_file(name, pin=None, return_pin=None):
    #Do allocation here, as we can't do it in an interrupt.
    frame = audio.AudioFrame()
    with open(name) as file:
        audio.play(audio_generator(file, frame), pin=pin, return_pin=return_pin)
