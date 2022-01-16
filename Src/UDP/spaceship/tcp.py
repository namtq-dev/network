import sysv_ipc as ipc
import pathlib
path1 = pathlib.Path(__file__).parent.resolve()

class CPythonConnection:
    def __init__(self):
        f = open(f'{path1}/../../../setting.txt')
        path = f.readlines()[2].replace("\n", "")
        self.key = ipc.ftok(path, 2332)
        self.shm = ipc.SharedMemory(self.key, 0, 0)
        self.xb = 0
        self.xr = 0
        self.yr = 0
        self.yb = 0
        self.n1 = 1000
        self.n2 = 1000
    
    def getData(self):
        self.shm.attach(0,0)
        buf = self.shm.read(100).decode("utf-8")
        positions = buf.split()
        if len(positions) > 2 :
            if len(positions[2]) > 0 :
                self.xr= int(positions[2])
                self.yr= int(positions[3])
                self.n2 = int(positions[5])
        return self.xr-2000, self.yr-2000, self.n2

    def writeData(self, x, y, n):
        s = f'{x+2000}      {y+2000}      {self.xr}      {self.yr}      {n}      {self.n2}      abc'
        self.shm.write(s)
        self.shm.detach()




