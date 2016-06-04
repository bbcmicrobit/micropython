from microbit import *
import random


class FallingBlocks:
    def __init__(self):
        self.screen = Image(5, 5)
        self.movingDot = None
        self.points = 0
        self.gameOver = False

    def addDot(self, i):
        if self.screen.get_pixel(i, 0) == 9:
            self.gameOver = True
        else:
            self.movingDot = (i, 0)
            self.screen.set_pixel(i, 0, 9)

    def stepDown(self):
        x, y = self.movingDot
        if y + 1 < 5 and self.screen.get_pixel(x, y + 1) == 0:
            self.screen.set_pixel(x, y, 0)
            self.screen.set_pixel(x, y + 1, 9)
            self.movingDot = (x, y + 1)
        else:
            self.movingDot = None

    def move(self, v):
        x, y = self.movingDot
        if x + v >= 0 and x + v < 5 and self.screen.get_pixel(x + v, y) == 0:
            self.screen.set_pixel(x, y, 0)
            self.screen.set_pixel(x + v, y, 9)
            self.movingDot = (x + v, y)

    def hasMovingDot(self):
        return self.movingDot != None

    def screenDown(self):
        self.screen = self.screen.shift_down(1)

    def checkLine(self):
        if all(self.screen.get_pixel(i, 4) == 9 for i in range(5)):
            self.points += 1
            self.screenDown()
            return True
        return False


while True:
    SPEED = 500
    SPEED2 = 200
    STEP = 10
    game = FallingBlocks()
    while not game.gameOver:
        if not game.hasMovingDot():
            game.addDot(random.randint(0, 4))
        display.show(game.screen)
        if not game.gameOver:
            t0 = running_time()
            while running_time() - t0 < SPEED:
                reading = accelerometer.get_x()
                if reading > 20:
                    game.move(1)
                elif reading < -20:
                    game.move(-1)
                display.show(game.screen)
                sleep(SPEED2)
            game.stepDown()
            if game.checkLine():
                if SPEED >= STEP:
                    SPEED -= STEP
                if SPEED2 >= STEP:
                    SPEED2 -= STEP

    while True:
        display.scroll("Game Over !")
        display.scroll(str(game.points) + " points")
        if button_a.was_pressed():
            break
