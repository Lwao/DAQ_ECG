from matplotlib.animation import FuncAnimation
import matplotlib.pyplot as plt
import numpy as np
import collections
import serial
import time

def animateFunc(i):
    if(len(buffer)==sizeWin):
        # get data
        [data_deque.popleft() for _i in range(sizeWin)]
        data_deque.extend(np.array(buffer))

        ax.cla() # clear axis
        ax.plot(data_deque, linewidth=.1) # plot ecg
        ax.set_xlabel('Samples')
        ax.set_ylabel('Amplitude')
        ax.set_title('Real-time ECG')
        ax.set_ylim(0,5000)

sizeWin = 1024
sizeDeque = sizeWin * 4
buffer = []
data_deque = collections.deque(np.zeros(sizeDeque)) # zero filled collection
fig, ax = plt.subplots(figsize=(10,4)) # config matplot figure
ani = FuncAnimation(fig, animateFunc, interval=1) # animate
plt.show() # display

ser = serial.Serial(port='COM3', 
                    baudrate=115200, 
                    timeout=1)
ser.flushInput()

while True:
    try:
        #bytesToRead = ser.inWaiting()
        ser_bytes = ser.read(2)
        try:
            decoded_bytes = float(ser_bytes[0:len(ser_bytes)-2].decode("utf-8"))
            buffer.append(decoded_bytes)
            #print(decoded_bytes)
            #print(np.array(buffer))
            if(len(buffer)==sizeWin):
               plt.show() # display
               buffer = []
        except:
            continue
    except:
        print("Keyboard Interrupt")
        break

    