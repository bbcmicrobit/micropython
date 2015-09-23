# this demo requires a speaker connected to P0 and GND

import microbit
from music import Tune

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
