import pygame
import pygame_gui
import math

class ListGamer:
    def __init__(self, screen):
        self.screen = screen
        self.manager = pygame_gui.UIManager((800, 600))
    
    def run(self ,list_user):
        running = True
        clock = pygame.time.Clock()
        background = pygame.Surface((800, 600))
        background.fill(pygame.Color('#000000'))
        list_of_buttons=[]

        def handle_button(index):
            if index >0:
                print(list_user[index-1])

        list_of_buttons.append(pygame_gui.elements.UIButton(relative_rect=pygame.Rect((0, 0), (80, 25)), text='tro lai', manager=self.manager))
        for i in range(len(list_user)):
            list_of_buttons.append(pygame_gui.elements.UIButton(relative_rect=pygame.Rect((100+150*(i%4), 50+(100*(int(math.floor(i/4))))), (100, 75)), text=list_user[i], manager=self.manager))
        
        while running:
            time_delta = clock.tick(60)/1000.0
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False
                if event.type == pygame.USEREVENT:
                    if event.user_type == pygame_gui.UI_BUTTON_PRESSED:
                        if event.ui_element in list_of_buttons:
                            index = list_of_buttons.index(event.ui_element)
                            if index==0:
                                running=False
                            else:
                                handle_button(index)
                self.manager.process_events(event)
            
            self.manager.update(time_delta)
            self.screen.blit(background, (0, 0))
            self.manager.draw_ui(self.screen)

            pygame.display.update()

