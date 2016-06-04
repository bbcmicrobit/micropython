from microbit import *
import random

class simpleTetris():
    
    def __init__(self):
        self.screen = [[0 for i in range(5)] for j in range(5)]
        self.movingDot = None
        self.points = 0
        self.gameOver = False
        
    def getImageStr(self):
        return ":".join("".join(str(v) for v in l) for l in self.screen) 
        
    def addDot(self,i):
        if self.screen[0][i] == 9:
            self.gameOver = True
        else:
            self.movingDot = (0,i)
            self.screen[0][i] = 9
    
    def stepDown(self):
        x,y = self.movingDot
        if x+1 < 5 and self.screen[x+1][y] == 0:
            self.screen[x][y] = 0
            self.screen[x+1][y] = 9
            self.movingDot = (x+1,y)
        else:
            self.movingDot = None
            
    def move(self,v):
        x,y = self.movingDot
        if y+v >= 0 and y+v < 5 and self.screen[x][y+v] == 0:
            self.screen[x][y] = 0
            self.screen[x][y+v] = 9
            self.movingDot = (x,y+v)
    
    def hasMovingDot(self):
        return self.movingDot != None
    
    def screenDown(self):
        for l in range(4,0,-1):
            self.screen[l] = self.screen[l-1]
        self.screen[0] = [0 for i in range(5)]
    
    def checkLine(self):
        if all(v == 9 for v in self.screen[-1]):
            self.points+=1
            self.screenDown()
            return True
        return False


while True:
    SPEED = 500
    SPEED2 = 200
    STEP = 10
    tetris = simpleTetris()
    while not tetris.gameOver:
        if not tetris.hasMovingDot():
            tetris.addDot(random.randint(0,4))
        display.show(Image(tetris.getImageStr()))
        if not tetris.gameOver:
            t0 = running_time()
            while running_time() - t0 < SPEED:
                reading = accelerometer.get_x()
                if reading > 20:
                    tetris.move(1)
                elif reading < -20:
                    tetris.move(-1)
                display.show(Image(tetris.getImageStr()))
                sleep(SPEED2)
            tetris.stepDown()
            if tetris.checkLine():
                if SPEED >= STEP:
                    SPEED -= STEP
                if SPEED2 >= STEP:
                    SPEED2 -= STEP

    while True:
        display.scroll("Game Over !")
        display.scroll(str(tetris.points) + " points")
        if button_a.was_pressed():
            break