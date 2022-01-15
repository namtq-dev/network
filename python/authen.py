from tkinter import *
import time
from communicate import Communicate

class AuthenForm:
  
    def handleSubmit(self):
        user = self.user_entry.get()
        password = self.password_entry.get()

        if self.login==True:
            self.communication.request(f"USER {user} {password}")
            for i in range(10):
                time.sleep(0.1)
                status,values=self.communication.checkRespond()
                if(status==True):
                    if(values[0]=='success\n'):
                        f = open('boundary/account.txt','w+')
                        f.write(user)
                        f.close()
                        self.login_result=True
                        self.root.destroy()
                        self.root.quit()
                    else:
                        self.announce_lb.configure(text='tai khoan hoac mat khau \n khong dung')
                    break
        else:
            self.communication.request(f"REGI {user} {password}")
            for i in range(10):
                time.sleep(0.1)
                status,values=self.communication.checkRespond()
                if(status==True):
                    if(values[0]=='success\n'):
                        self.announce_lb.configure(text='dang ki thanh cong')
                    else:
                        self.announce_lb.configure(text='username da ton tai')
                    break            


    
    def handleSwitch(self):
        if self.login==True:
           self.login = False
           self.login_label.configure(text='SIGN UP')
           self.switch_bt.configure(text='SIGN IN')
           self.a_lb.configure(text='Do you already have an account?')
        else:
           self.login = True
           self.login_label.configure(text='SIGN IN')
           self.switch_bt.configure(text='SIGN UP')
           self.a_lb.configure(text='you do not already have an account')
    
    def __init__(self):
        self.communication = Communicate()
        self.login=True
        self.login_result=False
        self.root = Tk()
        root = self.root
        self.root.title("Game")
        self.root.geometry('800x600')
        self.bg = PhotoImage(file = "background.png")
        self.label1 = Label( root, image = self.bg)
        self.label1.place(x = 0,y = 0)
        self.login_label = Label(root, text='SIGN IN' , font=('Arial', 40))
        self.login_label.place(x = 550,y = 120)
        self.user_label = Label(root, text='username' , font=('Arial', 15))
        self.user_label.place(x = 460,y = 200)
        self.user_entry = Entry(root, width=20, font=('Arial', 17))
        self.user_entry.place(x = 550,y = 200)
        self.password_label = Label(root, text='password' , font=('Arial', 15))
        self.password_label.place(x = 460,y = 250)
        self.password_entry = Entry(root, width=20, font=('Arial', 17))
        self.password_entry.place(x = 550,y = 250)
        self.login_button = Button(root,text="submit", font=('Arial', 20), command=self.handleSubmit)
        self.login_button.place(x = 580,y = 300)
        self.a_lb = Label(text='you do not already have an account', font=('Arial', 10))
        self.a_lb.place(x = 500,y = 350)
        self.switch_bt = Button(root,text="register", font=('Arial', 15), command=self.handleSwitch)
        self.switch_bt.place(x = 580,y = 370)
        self.announce_lb = Label(font=('Arial', 15))
        self.announce_lb.place(x = 500,y = 400)

    def authenticate(self):
        self.root.mainloop()
        return self.login_result

    
