# accompanying demo in examples

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
        None: 0
    }

    ALIASES = {
        'Db': 'C#',
        'Eb': 'D#',
        'Gb': 'F#',
        'Ab': 'G#',
        'Bb': 'A#'
    }

    def __init__(self, pin, bpm=180, notes=[]):
        self.pin = pin
        self.bpm = bpm

        self._notes = []
        self.add_notes(notes)
        # (1/frequency (in us), time)

    def play(self):
        self.pin.set_analog_value(100)

        for i, note in enumerate(self._notes):
            print("playing note %i of %i" % (i, len(self._notes)))

            period, length = note

            self.pin.set_analog_period_us(int(period))

            microbit.sleep(int(length * (60000//self.bpm)))

        self.stop()

    def stop(self):
        self.pin.set_analog_value(0)

    def add_note(self, note, length, octave = 0):
        if note in Tune.ALIASES:
            # this allows us to treat flats as sharps
            note = Tune.ALIASES[note]

        self._notes.append((Tune.NOTES[note] // (2 ** octave), length))

    def add_notes(self, notes):
        for note in notes:
            # the tuples in note can be just like the args of add_note
            self.add_note(*note)

HAPPY_BIRTHDAY = [
    ('C', 0.75),
    (None, 0.05), # so you can distinguish the two C's
    ('C', 0.25),
    ('D', 1),
    ('C', 1),
    ('F', 1),
    ('E', 2),
    ('C', 0.75),
    (None, 0.05), # so you can distinguish the two C's
    ('C', 0.25),
    ('D', 1),
    ('C', 1),
    ('G', 1),
    ('F', 2),
    ('C', 0.75),
    (None, 0.05), # so you can distinguish the two C's
    ('C', 0.25),
    ('C', 1, 1), # an octave above middle C :)
    ('A', 1),
    ('F', 1),
    ('E', 1),
    ('D', 2), # unfortunately we don't have pauses yet ;)
    ('Bb', 0.75),
    (None, 0.05),
    ('Bb', 0.25),
    ('A', 1),
    ('F', 1),
    ('G', 1),
    ('F', 3),
    (None,3),
    ('F', 1),
    ('C',0.33333),
    ('B',0.33333, -1),
    ('C',0.33333),
    ('C#',1),
    ('C',1),
    (None,1),
    ('E',1),
    ('F',1)
]
