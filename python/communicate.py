import time
class Communicate:
    def request(self, mess):
        f = open('boundary/request.txt','w+') 
        f.write("1\n")
        f.write(mess)
        f.close()
        f1 = open('boundary/respond.txt','w+')
        f1.write("0\n")
        f1.close()
    
    def checkRespond(self):
        status = False
        f = open('boundary/respond.txt','r')
        values = []
        data = f.readlines()
        print(data)
        if data[0] == '0\n':
            pass
        else:
            status = True
            values = data[1:]

        f.close()
        f = open('boundary/respond.txt','w')
        f.write("0\n")
        f.close()
        return status, values

    def checkChallenge(self):
        status = False
        f = open('boundary/challenge.txt','r')
        values = []
        data = f.readlines()
        if data[0] == '0\n':
            pass
        else:
            status = True
            values = data[1:]
        f.close()
        return status, values
    
    def resChallenge(self, mess):
        f = open('boundary/challenge.txt','w')
        f.close()
        f = open('boundary/accept.txt','w')
        f.write(mess)
        f.close()



