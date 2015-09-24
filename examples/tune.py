"""
This demo requires a speaker connected to P0 and GND
"""
import microbit


class Tune:
    """
    An instance of this class represents a melody that can be played.
    """

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

    def __init__(self, pin, bpm=480):
        self.pin = pin
        self.bpm = bpm
        self.notes = []
        self.default_length = 1

    def play(self):
        self.pin.set_analog_value(100)
        for note in self.notes:
            period, length = note
            self.pin.set_analog_period_us(period)
            microbit.sleep(length * (60000//self.bpm))
        self.pin.set_analog_value(0)

    def play_note(self, note, octave=0, length=None):
        if length == None:
            length = self.default_length
        else:
            self.default_length = length
        note = note.upper()
        if note in Tune.ALIASES:
            note = Tune.ALIASES[note]
        period = Tune.NOTES[note] // (2 ** octave)
        self.pin.set_analog_period_us(period)
        microbit.sleep(length * (60000//self.bpm))

    def add_note(self, note, octave=0, length=None):
        if length == None:
            length = self.default_length
        else:
            self.default_length = length
        note = note.upper()
        if note in Tune.ALIASES:
            note = Tune.ALIASES[note]
        self.notes.append((Tune.NOTES[note] // (2 ** octave), length))


# play Bach Prelude in C.
notes = [
    'c1', 'e1', 'g1', 'c2', 'e2', 'g1', 'c2', 'e2', 'c1', 'e1', 'g1', 'c2', 'e2', 'g1', 'c2', 'e2', 
    'c1', 'd1', 'g1', 'd2', 'f2', 'g1', 'd2', 'f2', 'c1', 'd1', 'g1', 'd2', 'f2', 'g1', 'd2', 'f2',
    'b0', 'd1', 'g1', 'd2', 'f2', 'g1', 'd2', 'f2', 'b0', 'd1', 'g1', 'd2', 'f2', 'g1', 'd2', 'f2',
    'c1', 'e1', 'g1', 'c2', 'e2', 'g1', 'c2', 'e2', 'c1', 'e1', 'g1', 'c2', 'e2', 'g1', 'c2', 'e2',
    'c1', 'e1', 'a1', 'e2', 'a2', 'a1', 'e2', 'a1', 'c1', 'e1', 'a1', 'e2', 'a2', 'a1', 'e2', 'a1',
    'c1', 'd1', 'f#1', 'a1', 'd2', 'f#1', 'a1', 'd2', 'c1', 'd1', 'f#1', 'a1', 'd2', 'f#1', 'a1', 'd2',
    'b0', 'd1', 'g1', 'd2', 'g2', 'g1', 'd2', 'g2', 'b0', 'd1', 'g1', 'd2', 'g2', 'g1', 'd2', 'g2',
    'b0', 'c1', 'e1', 'g1', 'c2', 'e1', 'g1', 'c2', 'b0', 'c1', 'e1', 'g1', 'c2', 'e1', 'g1', 'c2',
    'b0', 'c1', 'e1', 'g1', 'c2', 'e1', 'g1', 'c2', 'b0', 'c1', 'e1', 'g1', 'c2', 'e1', 'g1', 'c2',
    'a0', 'c1', 'e1', 'g1', 'c2', 'e1', 'g1', 'c2', 'a0', 'c1', 'e1', 'g1', 'c2', 'e1', 'g1', 'c2',
    'd0', 'a0', 'd1', 'f#1', 'c2', 'd1', 'f#1', 'c2', 'd0', 'a0', 'd1', 'f#1', 'c2', 'd1', 'f#1', 'c2',
    'g0', 'b0', 'd1', 'g1', 'b1', 'd1', 'g1', 'b1', 'g0', 'b0', 'd1', 'g1', 'b1', 'd1', 'g1', 'b1'
]

duration = 1
tune = Tune(microbit.io.P0)
tune.pin.set_analog_value(100)
for note in notes:
    tune.play_note(note[:-1], int(note[-1]), duration)
tune.pin.set_analog_value(0)
