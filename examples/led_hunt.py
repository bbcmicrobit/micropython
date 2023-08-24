"""
LED Hunt

Author: Piero Cornice

- Capture the faint LED with the bright one by tilting the device.
- Do it 10 times to win the game.
- See how quick you've been, to compete with friends.
- Press either A or B to reset the game.
"""

from microbit import *
from random import randint
import music
import time


def normalize(value, input_range, output_range) -> float:
    '''
    Rescale x from its min/max range to a target range [a, b] using the formula:

        a + (x - min(x)) * (b - a) / (max(x) - min(x))
    '''
    return output_range[0] + ((value - input_range[0]) * (output_range[1] - output_range[0])) / (input_range[0] - input_range[1])

def animate(images, delay_ms=50):
    for img in images:
        display.show(Image(img))
        sleep(delay_ms)
    display.clear()


def initialize_game():
    global score, x_target, y_target, start_at
    music.play(music.JUMP_UP, wait=False)
    animate(animation_start)
    score = 0
    x_target, y_target = set_target()
    start_at = time.ticks_ms()


def set_target():
    x_target, y_target = 2, 2
    # make sure the target is not in the center, as we'll probably start from there
    while x_target == 2 and y_target == 2:
        x_target = randint(0, 4)
        y_target = randint(0, 4)
    return x_target, y_target


animation_win = [
    '00000:00600:06960:00600:00000',
    '00600:06960:69696:06960:00600',
    '06960:69696:96069:69696:06960',
    '69696:96069:60006:96069:69696',
    '96069:60006:00000:60006:96069',
    '60006:00000:00000:00000:60006']

animation_start = list(reversed(animation_win))

# Initialize game variables with dummy values
# and let initialize_game() do the rest.
score, x_target, y_target, start_at = 0, 0, 0, 0
initialize_game()

while True:
    if button_a.was_pressed() or button_b.was_pressed():
        initialize_game()

    x_acc, y_acc, _ = accelerometer.get_values()

    x_acc = min(max(x_acc, -500), 500)
    y_acc = min(max(y_acc, -500), 500)
    
    x_pos = int(abs(normalize(x_acc, [-500, 500], [0, 4])))
    y_pos = int(abs(normalize(y_acc, [-500, 500], [0, 4])))
    
    #print('X: ', x_acc, ' => ', x_pos)
    #print('Y: ', y_acc, ' => ', y_pos)

    if x_pos == x_target and y_pos == y_target:
        # Win! Set a new target
        music.play(music.BA_DING, wait=False)
        score += 1
        display.show(score)
        sleep(1000)

        if score <= 9:
            # End of match: set a new target
            animate(animation_win)
            x_target, y_target = set_target()
        else:
            # End of game
            # Show the elapsed time and restart the game
            end_at = time.ticks_ms()
            music.play(music.POWER_UP, wait=False)
            for _ in range(3):
                animate(animation_win)
            elapsed_time = time.ticks_diff(end_at, start_at) // 1000
            display.scroll(str(elapsed_time) + ' sec', delay=100)
            initialize_game()

    # Update the screen
    display.clear()
    display.set_pixel(x_target, y_target, 5)
    display.set_pixel(x_pos, y_pos, 9)
    sleep(100)

