# this demo requires a speaker connected to P0 and GND

import microbit

class Tune:
    # 1,000,000/frequncy
    NOTES = {
        'C':  3822,
        'C#': 3608,
        'D':  3405,
        'D#': 3214,
        'E':  3034,
        'F':  2863,
        'F#': 2703,
        'G':  2551,
        'G#': 2408,
        'A':  2273,
        'A#': 2145,
        'B':  2025,
    }

    ALIASES = {
        'Db': 'C#',
        'Eb': 'D#',
        'Gb': 'F#',
        'Ab': 'G#',
        'Bb': 'A#'
    }

    def __init__(self, pin, bpm=180):
        self.pin = pin
        self.bpm = bpm

        self.notes = []
        # (1/frequency (in us), time)

    def play(self):
        self.pin.set_analog_value(100)

        for i, note in enumerate(self.notes):
            print("playing note %i of %i" % (i, len(self.notes)))

            period, length = note

            self.pin.set_analog_period_us(period)

            microbit.sleep(length * (60000//self.bpm))

    def stop(self):
        self.pin.set_analog_value(0)

    def add_note(self, note, length, octave = 0):
        self.notes.append((Tune.NOTES[note] // (2 ** octave), length))


# play a C Major scale
tune = Tune(microbit.io.P0)

notes = ['C', 'D', 'E', 'F', 'G', 'A', 'B']

for note in notes:
    tune.add_note(note, 1)

tune.add_note('C', 1, 1)

for note in reversed(notes):
    tune.add_note(note, 1)

tune.play()
tune.stop()
