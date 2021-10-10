from matplotlib.animation import FuncAnimation
import matplotlib.pyplot as plt
import numpy as np
import collections
import serial
import csv

def plot():
    # get data
    [ECG_deque.popleft() for _i in range(sizeWin)]
    [HR_deque.popleft() for _i in range(sizeWin)]
    ECG_deque.extend(np.array(buffer_ECG))
    HR_deque.extend(np.array(buffer_HR))
    
    ax1.cla() # clear axis
    ax1.plot(ECG_deque, linewidth=1) # plot ecg
    ax1.set_xlabel('Samples')
    ax1.set_ylabel('Amplitude')
    ax1.set_title('Real-time ECG')
    ax1.set_ylim(-1,1)

    ax2.cla() # clear axis
    ax2.plot(HR_deque, linewidth=1) # plot ecg
    ax2.set_xlabel('Samples')
    ax2.set_ylabel('Heart rate (bpm)')
    ax2.set_ylim(0,300)

    #plt.show()
    plt.pause(0.01)

ser = serial.Serial(port='COM3', 
                    baudrate=115200, 
                    timeout=1)
ser.flushInput()

sizeWin = 1024
sizeDeque = sizeWin * 4
buffer_ECG = []
buffer_HR = []
ECG_deque = collections.deque(np.zeros(sizeDeque)) # zero filled collection
HR_deque = collections.deque(np.zeros(sizeDeque)) # zero filled collection
fig, (ax1, ax2) = plt.subplots(figsize=(10,4), nrows=2, ncols=1) # config matplot figure

while True:
    try:
        ser_bytes = ser.readline()
        try:
            #decoded_bytes = float(ser_bytes[0:len(ser_bytes)-2].decode("utf-8"))
            decoded_bytes = ser_bytes.decode("utf-8")[:-4]
            decoded_list = decoded_bytes.split('\t')
            buffer_ECG.append(float(decoded_list[0]))
            buffer_HR.append(float(decoded_list[1]))
            if(len(buffer_ECG)==sizeWin):
                np.savetxt('stored_ECG.csv', np.array(buffer_ECG), delimiter='\n')
                np.savetxt('stored_HR.csv', np.array(buffer_HR), delimiter='\n')
                plot()
                buffer_ECG = []
                buffer_HR = []    
        except:
            continue
        
    except:
        print("Keyboard Interrupt")
        break

    