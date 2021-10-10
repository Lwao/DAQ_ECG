# Libraries

from ecg_generator_example import *
import numpy as np
import plotly.express as px
import plotly.graph_objects as go
from plotly.subplots import make_subplots

# Generate ECG signal

time, ecg = ecg_generator(t_end=10)
t = time[time>=5]
ecg = ecg[time>=5]

# Windowing

dt = 1e-4 # sampling time (s)
winDur = 0.1 # window duration (s)
winSize = int(winDur/dt) # window size
signalSize = len(ecg) # signal size
overlapPer = 0  # overlap percentage
overlap = int(overlapPer*winSize) # overlap samples
winIdx = np.arange(start=0, stop=signalSize, step=winSize-overlap) # window indexes
if(winIdx[-1]!=signalSize): winIdx = np.append(winIdx, signalSize-1)

# Find peaks

threPer = 0.9 # threshold percentage
thre = threPer*max(ecg) # threshold
#thre = 1.3*np.sqrt(np.mean(ecg**2))
filt = ecg * (ecg>thre) # ecg data higher than threshold
dfilt = np.int32(np.append(0,np.diff(filt))>0) # amplitude is rising 
ddfilt = np.int32(np.append(np.diff(dfilt),0)<0) # detect transition between rising and decreasing
peak_idx = np.where(ddfilt>0) 

# Heart rate

heart_rate = 1/np.diff(t[peak_idx])

# Plots

fig = make_subplots(rows=2, cols=1, shared_xaxes=False)

fig.add_trace(
    go.Scatter(x=t, y=ecg, name='ECG'),
    row=1, col=1
)
fig.add_trace(
    go.Scatter(x=t[peak_idx], y=ecg[peak_idx], name='QRS-peak', mode='markers', marker=dict(color='#EF553B',size=5)),
    row=1, col=1
)
fig.add_trace(
    go.Scatter(x=t[peak_idx][:-1], y=60*heart_rate, name='Heart rate'),
    row=2, col=1
)

fig.update_layout(height=600, width=500, title_text='ECG - Heart rate in normal rhythm')
fig.update_yaxes(title_text="", showgrid=True)
fig.update_xaxes(title_text="Time (s)", showgrid=True)

fig['layout']['yaxis1']['title']='Amplitude'
fig['layout']['yaxis2']['title']='Heart rate (bpm)'

fig.show()