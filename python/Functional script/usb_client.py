from matplotlib.animation import FuncAnimation
import matplotlib.pyplot as plt
import numpy as np
import collections
import serial
import csv

def plot():
    # get data
    [data_deque.popleft() for _i in range(sizeWin)]
    data_deque.extend(np.array(buffer))
    
    ax.cla() # clear axis
    ax.plot(data_deque, linewidth=1) # plot ecg
    ax.set_xlabel('Samples')
    ax.set_ylabel('Amplitude')
    ax.set_title('Real-time ECG')
    ax.set_ylim(-1,1)

    #plt.show()
    plt.pause(0.01)

ser = serial.Serial(port='COM3', 
                    baudrate=115200, 
                    timeout=1)
ser.flushInput()

sizeWin = 1024
sizeDeque = sizeWin * 4
buffer = []
data_deque = collections.deque(np.zeros(sizeDeque)) # zero filled collection
fig, ax = plt.subplots(figsize=(10,4)) # config matplot figure

while True:
    try:
        ser_bytes = ser.readline()
        try:
            decoded_bytes = float(ser_bytes[0:len(ser_bytes)-2].decode("utf-8"))
            buffer.append(decoded_bytes)
            if(len(buffer)==sizeWin):
                np.savetxt('stored_data.csv', np.array(buffer), delimiter='\n')
                plot()
                buffer = []
                
        except:
            continue
        
    except:
        print("Keyboard Interrupt")
        break

    