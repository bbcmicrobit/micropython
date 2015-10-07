from microbit import *

hands = (Image.CLOCK12, Image.CLOCK1, Image.CLOCK2, Image.CLOCK3, 
         Image.CLOCK4, Image.CLOCK5, Image.CLOCK6, Image.CLOCK7, 
         Image.CLOCK8, Image.CLOCK9, Image.CLOCK10, Image.CLOCK11,
         )

centre_dot = Image("\n\n  X  \n\n\n")

def fiveticks():
    fivemins = 0
    hours = 0
    while True:
        yield  hands[fivemins]*0.3 + hands[hours]*0.7
        fivemins = (fivemins+1)%12
        hours = (hours + (fivemins == 0))%12
        
def ticks():
    on = True
    for face in fiveticks():
        for i in range(5):
            if on:
                yield face + centre_dot
            else:
                yield face - centre_dot
            on = not on

def animate(seq, delay, repeat=False):
    while True:
        for img in seq:
            display.print(img)
            sleep(delay)
        if not repeat:
            return

animate(ticks(), 1000)
