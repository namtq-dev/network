import pathlib
import pygame 
import time
from spaceship.tcp import CPythonConnection
import math
path = pathlib.Path(__file__).parent.resolve()

class BlueSpaceship:
    def __init__(self, screen):
        self.screen = screen
        self.blood = 100
        self.nBullet = 99999
        self.x = 0
        self.y = 0
        self.xb = 0
        self.yb = 0
        self.readly = True
        self.spaceship = pygame.image.load(f'{path}/icon/rocket.png')
        self.bullet = pygame.image.load(f'{path}/icon/bullet.png')
        self.speed = 4
        self.bspeed = 6
        self.xchange = 0
        self.ychange = 0


    def fire(self):
        if self.readly == True:
            self.readly = False
            self.xb = self.x
            self.yb = self.y
            self.nBullet -= 1
    
    def deleleBullet(self):
        self.readly = True
    
    def getPosition(self):
        return self.x,self.y

    def getPositionBullet(self):
        return self.xb,self.yb

    def display_blood(self, x,y,blood):
        pygame.draw.rect(self.screen, (255,0,0), pygame.Rect(x,y,100,10))
        pygame.draw.rect(self.screen, (0,255,0), pygame.Rect(x,y,blood,10))
        pygame.display.flip()    

    def run(self):
        self.x += self.xchange
        self.y += self.ychange
        if self.y >= 530:
            self.y = 530
        if self.y <= 400:
            self.y = 400
        if self.x <= 10:
            self.x = 10
        if self.x >= 790:
            self.x = 790
        self.screen.blit(self.spaceship, (self.x - 15, self.y - 15))
        self.display_blood(self.x-30, self.y+50, self.blood)
        if self.readly == False:
            self.yb -= self.bspeed
            if self.yb < 0:
                self.readly = True
            self.screen.blit(self.bullet, (self.xb, self.yb))





class RedSpaceship:
    def __init__(self, screen):
        self.screen = screen
        self.blood = 100
        self.nBullet = 99999
        self.x = 0
        self.y = 0
        self.xb = 0
        self.yb = 0
        self.readly = True
        self.spaceship = pygame.image.load(f'{path}/icon/rocket1.png')
        self.bullet = pygame.image.load(f'{path}/icon/bullet1.png')
        self.speed = 4
        self.bspeed = 6
    
    def fire(self):
        if self.readly == True:
            self.readly = False
            self.xb = self.x
            self.yb = self.y
            self.nBullet -= 1
    
    def deleleBullet(self):
        self.readly = True

    def getPositionSpace(self):
        return self.getPosition(self.x, self.y)

    def display_blood(self, x,y,blood):
        pygame.draw.rect(self.screen, (255,0,0), pygame.Rect(x,y,100,10))
        pygame.draw.rect(self.screen, (0,255,0), pygame.Rect(x,y,blood,10))
        # pygame.display.flip()

    def getPosition(self, x, y):
        x = 756 - x
        y = 540 - y
        return x,y

    def getPositionBullet(self):
        return self.getPosition(self.xb, self.yb)

    def run(self, x, y, nBullet):
        self.x = x
        self.y = y
        if self.nBullet > nBullet:
            self.fire()
        self.nBullet = nBullet
        self.screen.blit(self.spaceship, self.getPosition(self.x, self.y))
        xb,yb = self.getPosition(self.x, self.y)
        self.display_blood(xb-20, yb-10, self.blood)

        if self.readly == False:
            self.yb -= self.bspeed
            if self.yb < 0:
                self.readly = True
            self.screen.blit(self.bullet, self.getPosition(self.xb-10, self.yb))


class HNDgame:
    def __init__(self, screen):
        self.screen = screen
        self.background = pygame.image.load(f'{path}/icon/bg.jpg')
        self.winbackground = pygame.image.load(f'{path}/icon/win.jpg')
        self.losebackground = pygame.image.load(f'{path}/icon/lose.jpg')
        self.share = CPythonConnection()

    def distance(self, x, y):
        if math.sqrt((x[0]-y[0])**2 + (x[1]-y[1])**2) < 27:
            return True

    def run(self):
        clocks = pygame.time.Clock()
        running = True
        blueGamer = BlueSpaceship(self.screen)
        redGamer = RedSpaceship(self.screen) 
        self.loadbackground = pygame.image.load(f'{path}/icon/loading.jpg')
        self.screen.fill((0, 0, 0))
        self.screen.blit(self.loadbackground, (0, 0))
        pygame.display.update()
        time.sleep(3)
        while running:
            self.screen.fill((0, 0, 0))
            self.screen.blit(self.background, (0, 0))
            for event in pygame.event.get():
                # if event.type == pygame.QUIT:
                    # running = False
                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_LEFT:
                        blueGamer.xchange = -blueGamer.speed
                    if event.key == pygame.K_RIGHT:
                        blueGamer.xchange = blueGamer.speed
                    if event.key == pygame.K_DOWN:
                        blueGamer.ychange = blueGamer.speed
                    if event.key == pygame.K_UP:
                        blueGamer.ychange = -blueGamer.speed
                    if (event.key == pygame.K_SPACE):
                        blueGamer.fire()
                
                if (event.type == pygame.KEYUP):
                    if event.key == pygame.K_LEFT or event.key == pygame.K_RIGHT:
                        blueGamer.xchange  = 0
                    if event.key == pygame.K_DOWN or event.key == pygame.K_UP:
                        blueGamer.ychange = 0
                        
            xr , yr ,nr = self.share.getData()
            print(f'{xr} , {yr} ,{nr}, {redGamer.nBullet}')   
            redGamer.run(xr , yr ,nr)
            blueGamer.run()

            if blueGamer.readly==False:
               if self.distance(blueGamer.getPositionBullet(), redGamer.getPositionSpace()):
                   blueGamer.readly = True
                   redGamer.blood -= 20
                   if redGamer.blood < 5:
                       running = False
                       blueGamer.x = blueGamer.y = blueGamer.nBullet = 5000  
                       self.screen.fill((0, 0, 0))
                       self.screen.blit(self.winbackground, (0, 0))
                       pygame.display.update()
                       time.sleep(3)
            if redGamer.readly==False:
               if self.distance(redGamer.getPositionBullet(), blueGamer.getPosition()):
                   redGamer.readly = True
                   blueGamer.blood -= 20 
                   if blueGamer.blood < 5:
                       running = False 
                       blueGamer.x = blueGamer.y = blueGamer.nBullet = 5000  
                       self.screen.fill((0, 0, 0))
                       self.screen.blit(self.losebackground, (0, 0)) 
                       pygame.display.update()   
                       time.sleep(3)  
            self.share.writeData(blueGamer.x, blueGamer.y, blueGamer.nBullet)        
            pygame.display.update()
            clocks.tick(75)


# pygame.init()
# screen = pygame.display.set_mode((800, 600))
# game = HNDgame(screen)
# game.run()