from microbit import *

# Play the default Sound Effect
audio.play(audio.SoundEffect())

# Create a new Sound Effect and immediately play it
audio.play(audio.SoundEffect(
    freq_start=400,
    freq_end=2000,
    duration=500,
    vol_start=100,
    vol_end=255,
    wave=audio.SoundEffect.WAVE_TRIANGLE,
    fx=audio.SoundEffect.FX_VIBRATO,
    shape=audio.SoundEffect.SHAPE_LOG
))

# Play a Sound Effect instance, modify an attribute, and play it again
my_effect = audio.SoundEffect(
    freq_start=400,
    freq_end=2000,
)
audio.play(my_effect)
my_effect.duration = 1000
audio.play(my_effect)

# You can also create a new effect based on an existing one, and modify
# any of its characteristics via arguments
my_modified_effect = my_effect.copy()
my_modified_effect.wave = audio.SoundEffect.WAVE_NOISE
audio.play(my_modified_effect)

# Use sensor data to modify and play an existing Sound Effect instance
my_effect.duration = 600
while True:
    # int() might be temporarily neededhttps://github.com/microbit-foundation/micropython-microbit-v2/issues/121
    my_effect.freq_start = int(scale(accelerometer.get_x(), from_=(-2000, 2000), to=(0, 9999)))
    my_effect.freq_end = int(scale(accelerometer.get_y(), from_=(-2000, 2000), to=(0, 9999)))
    audio.play(my_effect)

    if button_a.is_pressed():
        # Button A silences the micro:bit
        speaker.off()
        display.show(Image("09090:00000:00900:09990:00900"))
        sleep(500)
    elif button_b.is_pressed():
        # On button B re-enable speaker & play an effect while showing an image
        speaker.on()
        audio.play(audio.SoundEffect(), wait=False)
        display.show(Image.MUSIC_QUAVER)
        sleep(500)

    sleep(150)
