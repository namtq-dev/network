import tkinter 
from tkinter import * 
from tkinter import messagebox

class MyAnnounce:
    def __init__(self):
        self.window = Tk()
        self.window.eval('tk::PlaceWindow %s center'% self.window.winfo_toplevel())
        self.window.withdraw()

    def showInvitation(self, message):
        if messagebox.askyesno( 'Question' ,message)==True:
            return True
        return False

    def confirmInvitation(self):
        if messagebox.askyesno( 'Question' ,"xác nhận thách đấu")==True:
            return True
        return False

    def exit(self):
        self.window.destroy()

