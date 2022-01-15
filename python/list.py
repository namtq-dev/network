import pygame
import pygame_gui
import math
pygame.init()

screen = pygame.display.set_mode((800, 600))


pygame.display.set_caption('Quick Start')
window_surface = pygame.display.set_mode((800, 600))
background = pygame.Surface((800, 600))
background.fill(pygame.Color('#000000'))
manager = pygame_gui.UIManager((800, 600))
def show():
    new_list = ['[0]', '[1]', '[2]', '[3]']

    list_of_buttons=[]

    for i in range(len(new_list)):
        list_of_buttons.append(pygame_gui.elements.UIButton(relative_rect=pygame.Rect((150, 75+(100*i)), (100, 75)), text=new_list[i], manager=manager))

    clock = pygame.time.Clock()
    is_running = True

    while is_running:
        time_delta = clock.tick(60)/1000.0
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                is_running = False

            if event.type == pygame.USEREVENT:
                if event.user_type == pygame_gui.UI_BUTTON_PRESSED:
                    if event.ui_element in list_of_buttons:
                        print(list_of_buttons.index(event.ui_element))
            manager.process_events(event)

        manager.update(time_delta)

        window_surface.blit(background, (0, 0))
        manager.draw_ui(window_surface)

        pygame.display.update()

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

G = ListGamer(screen)
G.run(['huy','nam','dat','huy1','nam1','dat1','huy2','nam2','dat2'])
show()
