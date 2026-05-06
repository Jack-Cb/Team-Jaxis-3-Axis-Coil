#for testing
import numpy as np
import socket
import time
import math
import threading


class internalTime:
    #Initialization, time starts at 0
    def __init__(self, duration, tickPeriod, mode):
        self.duration = duration #n number of ticks
        self.mode = mode #choose between continous(1) and finite(0) mode, duration is ignored in continous mode
        self.tickPeriod = tickPeriod
        self.stopT = threading.Event()
        self.thread1 = threading.Thread(target=self.runT, daemon=True)
        self.n = 0

    #Thread execution loop
    def runT(self):
        while not self.stopT.is_set() or CheckTime > duration:
            time.sleep(self.tickPeriod)
            self.TickForward()
            
    #Start the thread
    def start(self):
        self.thread1.start()

    #Stop the thread
    def stop(self):
        self.stopT.set()
        self.mode = 0

    #Increment time
    def TickForward(self):
        self.n += 1

    #Decrement time
    def TickBack(self):
        if(self.n <= 0):
            print("Can't go past time 0");
            return
        self.n -= 1

    #Reports the time
    def CheckTime(self):
        self.time = self.n * self.tickPeriod
        print(str(round(self.time, 5)) + " seconds")

    #Basically the clock without threading, lets you manually speed up and slow down
    def Clock(self, nticks):
        if(self.mode == 1):
            while True:
                time.sleep(self.tickPeriod)
                self.TickForward()

        #Clock forward or back by n ticks, can use to reverse or skip time (standard progression just passes 1) 
        for i in range (0, abs(nticks)):
            if(self.mode == 0 and nticks >= 0):
                time.sleep(self.tickPeriod) #consider if sleeping the process is the best aproach... if it's acting like a cleint and the dsp is a server it should be fine 
                self.TickForward()
            if(self.mode == 0 and nticks <= 0):
                time.sleep(self.tickPeriod) 
                self.TickBack()

class Sine: 
    def __init__(self,amplitude,frequency):
        self.amplitude = amplitude
        self.frequency = frequency

    def generateWaveform(self,n):
        self.x = round(self.amplitude * np.sin(self.frequency * n), 8)
        self.n = n

    def readValue(self):
        print(str(self.x) + " @ n = " + str(self.n))
        
    def returnValue(self):
        return self.x
   

Timer = internalTime(100, 0.1, 0)
Timer.start()
while True:
    X = Sine(1, 1/24)
    X.generateWaveform(Timer.n)
    print("Time = "+str(Timer.n)+" Sin = " + str(X.returnValue()))
    Timer.CheckTime()
    time.sleep(0.01)
