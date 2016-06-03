# Red Rain by Matt Land, June 2016
# ~~~~~~~~~~~
# This is a game for the BBC Microbit, where the player moves an 'umbrella'
# at the bottom of the LED matrix, using the A and B buttons.
# Rain drops fall from the top of the LED matrix, and can be reflected
# when the player successfully blocks them using the umbrella.
#
# Game speed will increase after the player completes each level. To complete
# a level, block ten rain drops with the umbrella before missing five.
# Missing five drops on one level will cause the game to end.
#
# Scoring is awarded based on successfully blocking rain drops.
# Points are awarded based on current level difficulty, as well as the length
# of the current 'streak' of successful reflections. The length of the streak does
# not reset between levels; a long streak is the best way to maximize player score.
#
# Good luck!

from microbit import *
import random

# sprite store for the paddles
paddle_sprites = [
    "66000",
    "06600",
    "00660",
    "00066",
]
# where the paddle is on screen
paddle_position = 0


def run_paddle():
    # update the paddle position based on player input
    global paddle_position
    if button_a.is_pressed() and paddle_position > 0:
        paddle_position -= 1
    if button_b.is_pressed() and paddle_position < 3:
        paddle_position += 1


# sprite store for the balls
ball_sprites = [
    "90000:",
    "09000:",
    "00900:",
    "00090:",
    "00009:",
]


class ScoreState(object):
    # this object keeps score and increases difficulty of levels
    _hits = 0
    _misses = 0
    _level = 1
    time = 130
    score = 0
    _streak = 0

    def hit(self):
        # register a hit
        self._hits += 1
        self._streak += 1
        self.score += self._streak * self._level * 10
        display.show(Image.SQUARE)
        sleep(25)

    def miss(self):
        # register a miss
        self._misses += 1
        self._streak = 0
        display.show(Image.NO)
        sleep(250)

    def level_complete(self):
        if self._hits < 10:
            return
        display.show(Image.YES)
        sleep(300)
        display.clear()
        sleep(500)
        display.show(image)
        sleep(600)
        self._level += 1
        self.time -= 5
        self._misses = 0
        self._hits = 0
        display.scroll("Level {} Score {}".format(self._level, self.score), delay=75)

    def game_is_over(self):
        if self._misses >= 5:
            return True
        return False


class BallState(object):
    # this tracks and controls the drops that fall from top of screen

    _spawned = False
    _direction = False
    _column = 0
    _row = 0
    score = None

    def _spawn(self):
        if not self._spawned:
            self._column = random.randint(0, 4)
            self._row = -1
            self._spawned = True
            self._direction = False

    def _move(self):
        # falling ball in the field
        if not self._direction and my_ball._row <= 3:
            self._row += 1
            return
        # rising ball
        if self._direction and my_ball._row > -1:
            self._row -= 1
            return

    def _test_collision(self):
        if self._row != 4:  # the ball is not low enough yet
            return
        if self._column == paddle_position or self._column == paddle_position + 1:
            # player hit the ball
            self._hit()
        else:
            self._miss()

    def _despawn(self):
        # get rid of a ball that went off the top or bottom of field
        if not self._direction and my_ball._row == 4:
            self._spawned = False
        if self._direction and my_ball._row == -1:
            self._spawned = False

    def _hit(self):
        # the player hit the ball, change its direction and register the hit
        self._direction = True
        self.score.hit()

    def _miss(self):
        # the player missed the ball, handle that miss and score it
        self._despawn()
        self.score.miss()

    def run(self, score):
        # this is the public entry
        self._spawn()
        self._move()
        self._test_collision()
        self._despawn()

    # this method costs memory, only uncomment if needed
    # def __str__(self):
    #     print('spa: ' + str(self._spawned) + ' row: ' + str(self._row) + ' column: ' + str(self._column) + ' dir: ' + str(self._direction))

    def draw_field(self, row):
        # draw one row of the field
        if not self._spawned or not row is self._row:
            return "00000:"
        return ball_sprites[self._column]


my_score = ScoreState()
my_ball = BallState()
my_ball.score = my_score  # skipping using __init__ since functions cost memory

while True:
    run_paddle()
    my_ball.run(my_score)
    field = [my_ball.draw_field(n) for n in range(4)]
    field = "".join(field) + paddle_sprites[paddle_position]
    image = Image(field)
    display.show(image)
    my_score.level_complete()
    if my_score.game_is_over():
        break
    sleep(my_score.time)

display.scroll("Game Over Score {}".format(my_score.score), loop=True, delay=100)
