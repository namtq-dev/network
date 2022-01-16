import pygame
import pygame_menu
import pygame_gui
import math
import time

from communicate import Communicate
from authen import AuthenForm
from diaglog import MyAnnounce

pygame.init()


class Arena:
    def __init__(self, screen):
        self.background = pygame.image.load('image/bg.jpg')
        self.screen = screen
        self.manager = pygame_gui.UIManager((800, 600))
        self.communication = Communicate()
        self.announce = MyAnnounce()
        f = open('boundary/account.txt','r')
        self.account = f.readlines()[0]
        f.close()

    def run(self ,list_user):
        running = True
        clock = pygame.time.Clock()
        background = pygame.Surface((800, 600))
        background.fill(pygame.Color('#000000'))
        list_of_buttons=[]

        def deleteBtn(list=list_of_buttons):
            l = len(list)
            for bt in list:
                bt.kill()
            for i in range(l):
                list.pop()


        def handle_button(index, list_user):
            if index==1:
                self.communication.request(f"LIST {self.account}")
                for i in range(20):
                    time.sleep(0.1)
                    status,values=self.communication.checkRespond()
                    print(status, values)
                    if(status==True):
                        print("da cap nhat")
                        deleteBtn()
                        list_user.clear()
                        list_user += values[1:]
                        setGui()
            if index >1:
                print(list_user[index-2])
                print(list_of_buttons)
                if self.announce.confirmInvitation():
                    f = open("../setting.txt")
                    setting = f.readlines()                    
                    f.close()
                    print(setting)
                    setting[0] = setting[0].replace("\n", "")
                    print(setting[0])
                    setting[1] = setting[1].replace("\n", "")
                    peer = list_user[index-2].replace("\n", "")
                    self.communication.request(f'CHAL {self.account} {peer} {setting[0]} {setting[1]}')
                for i in range(30):
                    time.sleep(0.1)
                    status,values=self.communication.checkRespond()
                    if(status==True):
                        print(values)  
                        break                  
                #  do something



        def setGui(list_of_buttons=list_of_buttons):
            list_of_buttons.append(pygame_gui.elements.UIButton(relative_rect=pygame.Rect((0, 0), (80, 25)), text='tro lai', manager=self.manager))
            list_of_buttons.append(pygame_gui.elements.UIButton(relative_rect=pygame.Rect((320, 10), (120, 40)), text='cap nhat', manager=self.manager))
            for i in range(len(list_user)):
                list_of_buttons.append(pygame_gui.elements.UIButton(relative_rect=pygame.Rect((100+150*(i%4), 140+(100*(int(math.floor(i/4))))), (100, 75)), text=list_user[i], manager=self.manager))
        setGui()

        while running:
            time_delta = clock.tick(60)/1000.0
            chall,message = self.communication.checkChallenge()
            if chall:
                k = self.announce.showInvitation(f'co loi moi thach dau tu\n{message[0]}')
                if k == True:
                    info = message[0].split()
                    addr = info[2]
                    port = info[3].replace("\n", "")
                    f = open("boundary/peer.txt", "w")
                    f.write("1\n")
                    f.write(f'{addr} {port}')
                    f.close()   
                    f = open("../setting.txt")
                    setting = f.readlines()                    
                    f.close()
                    print(setting)
                    setting[0] = setting[0].replace("\n", "")
                    print(setting[0])
                    setting[1] = setting[1].replace("\n", "")
                    self.communication.resChallenge(f'RESP\naccept {setting[0]} {setting[1]}')
                    # do some thing
                else:
                    self.communication.resChallenge('RESP\nreject x x')
        
            if False:
                pass
            else:
                for event in pygame.event.get():
                    if event.type == pygame.USEREVENT:
                        if event.user_type == pygame_gui.UI_BUTTON_PRESSED:
                            if event.ui_element in list_of_buttons:
                                index = list_of_buttons.index(event.ui_element)
                                if index==0:
                                    self.communication.request(f'LEAV {self.account}')
                                    for i in range(10):
                                        self.communication.checkRespond()
                                    running=False
                                else:
                                    handle_button(index, list_user)
                    self.manager.process_events(event)
            self.manager.update(time_delta)
            self.screen.blit(background, (0, 0))
            self.screen.blit(self.background, (0, 0))

            self.manager.draw_ui(self.screen)

            pygame.display.update()   

class Rank:
    def __init__(self, screen):
        self.background = pygame.image.load('image/bg.jpg')
        self.screen = screen
        self.manager = pygame_gui.UIManager((800, 600))
        self.communication = Communicate()
        self.announce = MyAnnounce()
        f = open('boundary/account.txt','r')
        self.account = f.readlines()[0]
        f.close()

    def showRank(self):
        running = True
        clock = pygame.time.Clock()
        background = pygame.Surface((800, 600))
        background.fill(pygame.Color('#000000'))
        list_of_buttons=[]
        myscore = 0
        topGamers = []
        mWin = 0
        mLoss = 0
        self.communication.request(f"RANK {self.account}")
        for i in range(10):
            time.sleep(0.1)
            status,values=self.communication.checkRespond()
            if(status==True):
                myscore = values[1]
                topGamers = values[2:]
                mWin = myscore.split()[0]
                mLoss = myscore.split()[1]
                break
        list_of_buttons.append(pygame_gui.elements.UIButton(relative_rect=pygame.Rect((0, 0), (80, 25)), text='tro lai', manager=self.manager))
        pygame_gui.elements.UIButton(relative_rect=pygame.Rect((200, 140), (200, 25)), text=f'win {mWin} loss{mLoss}', manager=self.manager)
        print(topGamers)
        for i in range(len(topGamers)):
            info = topGamers[i].split()
            usern = info[0]
            win = info[1]
            loss = info[2]
            rank = info[3]
            pygame_gui.elements.UIButton(relative_rect=pygame.Rect((200, 140+50*i), (200, 40)), text=f'{usern} win {win} loss {loss} rank{rank}', manager=self.manager)
        while running:
            time_delta = clock.tick(60)/1000.0
            for event in pygame.event.get():
                    if event.type == pygame.USEREVENT:
                        if event.user_type == pygame_gui.UI_BUTTON_PRESSED:
                            if event.ui_element in list_of_buttons:
                                index = list_of_buttons.index(event.ui_element)
                                if index==0:
                                    running=False

                    self.manager.process_events(event)
            self.manager.update(time_delta)
            self.screen.blit(background, (0, 0))
            self.screen.blit(self.background, (0, 0))
            self.manager.draw_ui(self.screen)
            pygame.display.update()
        




        

                



def display_blood(x,y,blood):
    pygame.draw.rect(screen, (255,0,0), pygame.Rect(x,y,120,10))
    pygame.draw.rect(screen, (0,255,0), pygame.Rect(x,y,blood,10))
    pygame.display.flip()



class MenuGame:
    def __init__(self, screen):
        self.screen = screen
        self.communication = Communicate()
        f = open('boundary/account.txt','r')
        self.account = f.readlines()[0]
        f.close()

    def showListScore(self):
        # myscore = 0
        # topGamers = []
        # self.communication.request(f"RANK {self.account}")
        # for i in range(10):
        #     time.sleep(0.1)
        #     status,values=self.communication.checkRespond()
        #     if(status==True):
        #         myscore = values[1]
        #         topGamers = values[2:]
        #         break

        # list_score = pygame_menu.Menu('Ranking', 800, 600, theme=pygame_menu.themes.THEME_BLUE)
        # list_score.add.label(f'Thành tích của bạn {myscore}')
        # list_score.add.label('TOP RANKING')
        # list_score.add.label('Name Win Lose Rank')
        # for i in range(len(topGamers)):
        #     list_score.add.label(f'{topGamers[i]} ')
        # return list_score
        rank = Rank(self.screen)
        rank.showRank()

    def goArena(self):
        G = Arena(self.screen)
        self.communication.request(f"LIST {self.account}")
        for i in range(10):
            time.sleep(0.1)
            status,values=self.communication.checkRespond()
            if(status==True):
                G.run(values[1:])
                break        


    def gameExit(self):
        exit()

    def run(self):
        menu = pygame_menu.Menu('Welcome', 800, 600, theme=pygame_menu.themes.THEME_BLUE)
        menu.add.button('Đến đấu trường', self.goArena)
        menu.add.button('Điểm số',self.showListScore)
        menu.add.button('Thoát Game', self.gameExit)
        menu.mainloop(screen)



authen = AuthenForm()
result = authen.authenticate()
if result:
    screen = pygame.display.set_mode((800,600))
    running = True
    menu = MenuGame(screen)
    menu.run()