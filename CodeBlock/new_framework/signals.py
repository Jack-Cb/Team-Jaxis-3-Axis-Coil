import numpy as np
import copy
import array
import socket
import time
import math
import sys
import threading

#For IPC with the DSP server
class Client:
    def __init__(self):
        self.SignalSocket = None
        self.HostAddress = "127.0.0.1"
        self.Port = 20001
        self.bufferSize = 16
       
    def Connect(self):
        self.soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        try:
            self.soc.connect((self.HostAddress, self.Port))
            print(f"Connected to {self.HostAddress} @ Port:{self.Port}")
        except Exception:
            print(f"Connection failed: {Exception}")
            self.soc = None

    def Send(self, package):
        if(self.soc):
            self.soc.sendall(package)
        else:
            print("Failed to send package, no socket")

    def Close(self):
        if(self.soc):
            self.soc.close()
            self.soc = None
            print("Closing socket")

    def Recieve(self):
        return


#Clock
class InternalTime:
    #Initialization, time starts at 0
    def __init__(self, duration, tickPeriod, mode):
        self.duration = duration #n number of ticks
        self.mode = mode #choose between continous(1) and finite(0) mode, duration is ignored in continous mode
        self.tickPeriod = tickPeriod
        self.stopT = threading.Event()
        self.thread1 = threading.Thread(target=self.RunT, daemon=True)
        self.n = 0.0

    #Thread execution loop
    def RunT(self):
        while not self.stopT.is_set() and (self.CheckTime() < self.duration or self.mode == 1):
            time.sleep(self.tickPeriod)
            self.TickForward()

    #Check thread status
    def Alive(self):
        return self.thread1.is_alive()

    #Start the thread
    def Start(self):
        self.thread1.start()

    #Stop the thread
    def Stop(self):
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
        return self.time
    #Reads the time 
    def ReadTime(self):
        print("Time lapsed: " + str(round(self.time, 5)) + " seconds")
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

    
#Parent signal class
class Signal:
    def __init__(self,amplitude,frequency):
        self.amplitude = amplitude
        self.frequency = frequency

    def ReadValue(self):
        print(f"{self.x} @ n = {self.n}")
    
    def ReturnValue(self):
        return self.x

#Sine wave
class Sine(Signal): 
    def GenerateWaveform(self,n):
        self.x = round(self.amplitude * np.sin(self.frequency * n), 8)
#       self.x = float(self.amplitude * np.sin(self.frequency * n))
        self.n = n

#Square wave
class Square(Signal):
    def __init__(self, amplitude, frequency, duty, tickPeriod):
        super().__init__(amplitude,frequency)
        self.duty = duty
        self.tickPeriod = tickPeriod
    def GenerateWaveform(self,n,duty):
        period = 1/self.frequency
        TickPerPeriod = period/self.tickPeriod
        OnTicks = TickPerPeriod * self.duty
        Frame = n % TickPerPeriod
        if(Frame <= OnTicks): 
            self.x = self.amplitude
        else:
            self.x = 0
        self.n = n

#Triangle Wave (later if needed)
#class Triangle(Signal):
 #   def GenerateWaveform(self,n):
  #      return

#Stores a group of 3 signals of same time.
class SignalSet():
    def __init__(self,X,Y,Z,tickPeriod):
        self.X = X
        self.Y = Y
        self.Z = Z
        self.tickPeriod = tickPeriod
    

    def ReadSet(self):
        print("X WAVE: ", end = "") 
        self.X.ReadValue()
        print("Y WAVE: ", end = "")
        self.Y.ReadValue()
        print("Z WAVE: ", end = "")
        self.Z.ReadValue()
  #      time.sleep(self.tickPeriod) #accounts for delay in ADC sample switching from X -> Y -> Z
  #      self.Y.GenerateWaveform(n + 1) #this is NOT SAFE. The timer clock thread may drift from this thread, but functions as an approximation
   #     time.sleep(self.tickPeriod)
    #    self.Z.GenerateWaveform(n + 2)
    #    time.sleep(self.tickPeriod)

#Group of 3 Sine waves
class SinSet(SignalSet):
    def __init__(self,Xamp,Xfreq,Yamp,Yfreq,Zamp,Zfreq,tickPeriod):
        super().__init__(
            X = Sine(Xamp,Xfreq),
            Y = Sine(Yamp,Yfreq),
            Z = Sine(Zamp,Zfreq),
            tickPeriod = tickPeriod
        )
    def Cycle(self, n):
        self.X.GenerateWaveform(n)
#        time.sleep(self.tickPeriod)
        self.Y.GenerateWaveform(n+1)
 #       time.sleep(self.tickPeriod)
        self.Z.GenerateWaveform(n+2)
  #      time.sleep(self.tickPeriod)

#Group of 3 Square Waves
class SquareSet(SignalSet):
    def __init__(self,Xamp,Xfreq,Xduty,Yamp,Yfreq,Yduty,Zamp,Zfreq,Zduty,tickPeriod):
        super().__init__(
            X = Square(Xamp,Xfreq,Xduty,tickPeriod),
            Y = Square(Yamp,Yfreq,Yduty,tickPeriod),
            Z = Square(Zamp,Zfreq,Zduty,tickPeriod),
            tickPeriod = tickPeriod
        )

    def Cycle(self, n):
        self.X.GenerateWaveform(n, self.X.duty)
   #     time.sleep(self.tickPeriod)
        self.Y.GenerateWaveform(n + 1, self.Y.duty) #this is NOT SAFE. The timer clock thread may drift from this thread, but functions as an approximation
    #    time.sleep(self.tickPeriod)
        self.Z.GenerateWaveform(n + 2, self.Z.duty)
     #   time.sleep(self.tickPeriod)           
        
        #not important right now
#class TriangleSet(SignalSet):
 #    def __init__(self,Xamp,Xfreq,Yamp,Yfreq,Zamp,Zfreq,tickPeriod):
  #      super().__init__(
   #         X = Triangle(Xamp,Xfreq),
    #        Y = Triangle(Yamp,Yfreq),
     #       Z = Triangle(Zamp,Zfreq),
      #      tickPeriod = tickPeriod
       # )   
#class TriangleSet:


#Represents 3 signals as a unit, stores time, and runs different modes
class Coil:
    def __init__(self, select, sigMode, timeMode):
        self.modes = { 
            "CustomSet": self.CustomSet,
            "RandomConditions": self.RandomConditions,
            "Octant": self.Octant,
            "LocationScan": self.LocationScan,
            "IndivSweep": self.IndivSweep,
#Sweeps each wave from min to max in parallel
            "SumulSweep": self.SimulSweep
        }
        self.TimeMode = timeMode
        self.SigMode = None
        self.Duration = None
        self.TickPeriod = None
        self.Noise = None
        self.n = 0.0
        self.select = select
        self.Waves = None

    def Run(self, *args, **kwargs):
        return self.modes[self.SigMode](*args, **kwargs)

    def Read(self):
        self.Waves.ReadSet()
        return

    def Update(self):
        self.Waves.Cycle(self.n)
        return
    
    def Output(self):
        #3 bytes in one package [x, y, z]
        #use array for precise sizing
        if(self.Waves == None):
            print("Nothing to output")
            return
        package = array.array('f', [self.Waves.X.ReturnValue(), self.Waves.Y.ReturnValue(), self.Waves.Z.ReturnValue(), self.n, self.TickPeriod])
        return package


    #Choose from sine, square, or triangle
    #Could probably make this an array but I'm lazy
    def signalSelect(self,XAMP,XFREQ,XDUTY,YAMP,YFREQ,YDUTY,ZAMP,ZFREQ,ZDUTY):
         if(self.select == "Sine"):
             self.Waves = SinSet(XAMP,XFREQ,YAMP,YFREQ,ZAMP,ZFREQ,self.TickPeriod)
         elif(self.select == "Square"):
             self.Waves = SquareSet(XAMP,XFREQ,XDUTY,YAMP,YFREQ,YDUTY,ZAMP,ZFREQ,ZDUTY,self.TickPeriod)
         elif(self.select == "Triangle"):
             pass
         else:
             print("Failure to assign a waveform")
             self.Waves = 0
         return
        

    #Allows for custom configuration of each signal
    def CustomSet(self):
        X_A = input("Please enter a X amplitude: ")
        XAMP = eval(X_A, {"pi":np.pi})
        X_F = input("Please enter a X frequency: ")
        XFREQ = eval(X_F, {"pi":np.pi})
        if(self.select == "Square"):
            XDUTY = float(input("Please enter an X duty cycle: "))
        else:
            XDUTY = 0
        Y_A = input("Please enter a Y amplitude: ")
        YAMP = eval(Y_A, {"pi":np.pi})
        Y_F = input("Please enter a Y frequency: ")
        YFREQ = eval(Y_F, {"pi":np.pi})
        if(self.select == "Square"):
            YDUTY = float(input("Please enter an Y duty cycle: "))
        else:
            YDUTY = 0
        Z_A = input("Please enter an Z amplitude: ")
        ZAMP = eval(Z_A, {"pi":np.pi})
        Z_F = input("Please enter an Z frequency: ")
        ZFREQ = eval(Z_F, {"pi":np.pi})
        if(self.select == "Square"):
            ZDUTY = float(input("Please enter an Z duty cycle: "))
        else:
            ZDUTY = 0
        self.signalSelect(XAMP,XFREQ,XDUTY,YAMP,YFREQ,YDUTY,ZAMP,ZFREQ,ZDUTY)

        print("\n\n")
        return

    #Updates the state of the clock. Used for waveform generation at time n
    def UpdateTime(self, n):
        self.n = n
        return

    #Randomizes signal values
    def RandomConditions():
        print("test")
        return

    #Attempts to generate waveforms matching an expected octant
    def Octant():
        return

    #Attepts to generate waveforms matching an expect <X,Y,Z> location
    def LocationScan():
        return

    #Sweeps each wave from min to max in succession X ---> Y ---> Z --->
    def IndivSweep():
        return

    #Sweeps each wave from min to max in parallel
    def SimulSweep():
        return

#Represents the coil but with noise added in
class NoiseCoil(Coil):
    def __init__(self):
        return

    #Signal Gen + Noise Generation
    #takes a noise argument from 0 to 1. 0 being no noise, 1 being max noise
    def SimNoise(self):
        #Gaussian
        #Mean 0, sigma = amplitude * noise scale
        XNoise = np.random.normal(0,self.Waves.X.amplitude * self.Noise, 1)
        YNoise = np.random.normal(0,self.Waves.Y.amplitude * self.Noise,1 )
        ZNoise = np.random.normal(0,self.Waves.Z.amplitude * self.Noise,1)
        #flicker?

        self.Waves.X.x += XNoise
        self.Waves.Y.x += YNoise
        self.Waves.Z.x += ZNoise
        return

def Prompt(CoreCoil):
    print("Signal Generation Simulation Program v1.0 (Case Sensitive)")
    print("Please select one of the following. Three signals of that type will be simulated")
    CoreCoil.select = str(input("Sine, Square, Triangle: "))
    print("Please select a tick period (0.001 minimum)")
    print("Real conditions would be 0.2uS, but the sim conditions don't allow for less than a microsecond")   
    print("This defines the clock resolution for the program")
    CoreCoil.TickPeriod = float(input("Tick period(s): "))
    CoreCoil.TimeMode = int(input("Continous (1) or finite mode(0): "))
    if(not CoreCoil.TimeMode):
        print("Please select a duration(s)")
        CoreCoil.Duration = float(input("Duration: ")) 
    else:
        CoreCoil.Duration = 0
    print("Please select a mode from the following.")
    CoreCoil.SigMode = str(input("CustomSet, RandomConditions, Octant, LocationScan, IndivSweep, SimulSweep:  "))
    CoreCoil.Noise = float(input("Please input a noise scale from 0 (none) to 1 (absolute): "))
    print("\n\n")
    return
 

#Main loop
NoisedCoil = NoiseCoil()
SigClient = Client()
SigClient.Connect()
CoreCoil = Coil(0,0,0) #Create a coil object. Initial parameters don't really matter, will change later
Prompt(CoreCoil) #Fill attributes

ReadTime = 2

while ReadTime:
    print(f"Commencing in {ReadTime} seconds")
    time.sleep(1)
    ReadTime -= 1


Timer = InternalTime(CoreCoil.Duration,CoreCoil.TickPeriod,CoreCoil.TimeMode) #Create a clock
CoreCoil.Run() #Select for configurations
NoisedCoil.__dict__.update(copy.deepcopy(CoreCoil.__dict__))
#print("Size of data input to DSP code")
#print(f"X Bytes: {sys.getsizeof(NoisedCoil.Waves.X.ReturnValue())}")
#print(f"Y Bytes: {sys.getsizeof(NoisedCoil.Waves.Y.ReturnValue())}")
#print(f"Z Bytes: {sys.getsizeof(NoisedCoil.Waves.Z.ReturnValue())}")

Timer.Start() #Start the clock
client_id = 1
flag = True

while Timer.Alive(): 
    time.sleep(3*CoreCoil.TickPeriod) #Accounts for ADC alternating between directions by x3
    CoreCoil.UpdateTime(Timer.n)
    CoreCoil.Update()
    NoisedCoil.UpdateTime(Timer.n)
    NoisedCoil.Update()
    NoisedCoil.SimNoise()
#   print(CoreCoil.Output())
#    SigClient.Send(CoreCoil.Output())
    print("Client Side")
    CoreCoil.Read()
#    NoisedCoil.Read()
    if(flag):
        SigClient.Send(client_id)
        flag = False

    SigClient.Send(NoisedCoil.Output())
#    SigClient.Send(4);
#    CoreCoil.Read()
#    NoisedCoil.Read()


print(f"Data was input into DSP program at a rate of {1/((3*CoreCoil.TickPeriod)*1000000)} MB/s")
Timer.CheckTime()
Timer.ReadTime()
SigClient.Close()

