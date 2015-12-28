"""
A simple name generator to use in the classroom. 
It needs to be prepared with the names of the students and the class name.
Press A to get a random name. Press B to display the last name again.
By Marco Tranchino for the "BBC MicroBit with Micro Python" project.
2015-12-28
"""

from microbit import *

needles = Image.ALL_CLOCKS

group = "Year 12"
pupilslist = ["Alex", "Ben", "Charlotte", "Daniel", "Eve", "Frank", "Gabrielle", "Henry", "Ines", "Jasmine", "Kaleb"   ]

pupils = pupilslist.copy()
pupil = "Press A"
message = "Random Name Generator for "+group+"... Press A"
display.scroll(message)


while True:
    if len(pupils) == len(pupilslist):
        while not (button_a.is_pressed() or button_b.is_pressed()):
            for i in range(12):
                display.show(needles[i])
                sleep(20)
        
    if button_a.is_pressed():
        if len(pupils) == 0:
            pupils = pupilslist.copy()  
            restarting = "Restarting!"
            display.scroll(restarting)
        else:
            i = random(len(pupils))
            pupil = pupils.pop(i)
            display.scroll(pupil)
    if button_b.is_pressed():
         display.scroll(pupil)  
    sleep(100)
