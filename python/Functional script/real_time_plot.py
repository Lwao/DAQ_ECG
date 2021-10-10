from ecg_generator import *
from matplotlib.animation import FuncAnimation
import matplotlib.pyplot as plt
import numpy as np
import collections

def animateFunc(i):
    # get data
    [data_deque.popleft() for _i in range(sizeWin)]
    [time_deque.popleft() for _i in range(sizeWin)]
    data_deque.extend(ecg[i*sizeWin:(i+1)*sizeWin])
    time_deque.extend(time[i*sizeWin:(i+1)*sizeWin])

    ax.cla() # clear axis
    ax.plot(time_deque, data_deque, linewidth=1) # plot ecg
    ax.set_xlabel('Time (s)')
    ax.set_ylabel('Amplitude')
    ax.set_title('Real-time ECG')
    ax.set_ylim(-1,1)

sizeWin = 1024
sizeDeque = sizeWin * 32

time, ecg = ecg_generator(t_end=40) # ecg signal
data_deque = collections.deque(np.zeros(sizeDeque)) # zero filled collection
time_deque = collections.deque(np.zeros(sizeDeque)) # zero filled collection

fig, ax = plt.subplots(figsize=(10,4)) # config matplot figure
ani = FuncAnimation(fig, animateFunc, interval=1) # animate
plt.show() # display