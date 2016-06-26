from microbit import *

hands = Image.ALL_CLOCKS

#A centre dot of brightness 2.
ticker_image = Image("2\n").crop(-2,-2,5,5)

#Adjust these to taste
MINUTE_BRIGHT = 0.1111
HOUR_BRIGHT = 0.55555

#Generate hands for 5 minute intervals
def fiveticks():
    fivemins = 0
    hours = 0
    while True:
        yield  hands[fivemins]*MINUTE_BRIGHT + hands[hours]*HOUR_BRIGHT
        fivemins = (fivemins+1)%12
        hours = (hours + (fivemins == 0))%12

#Generate hands with ticker superimposed for 1 minute intervals.      
def ticks():
    on = True
    for face in fiveticks():
        for i in range(5):
            if on:
                yield face + ticker_image
            else:
                yield face - ticker_image
            on = not on

#Run a clock speeded up 60 times, so we can watch the animation.
for tick in ticks():
    display.show(tick)
    sleep(1000)
