import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

def ecg_generator(dt=1e-4, t_end=1):
    t = np.arange(0,t_end+dt,dt)
    x1, x2, x3, x4, ecg = ([0],[0],[0.1],[0],[])
    C, b = (1.35, 4)
    H, a1, a2, a3, a4, Gt = (3, -0.024, 0.0216, -0.0012, 0.12, 7) # normal rhythm
    #H, a1, a2, a3, a4, Gt = (2.848, 0, -0.1, 0, 0, 21) # sinus tachycardia
    #H, a1, a2, a3, a4, Gt = (1.52, -0.068, 0.028, -0.024, 0.12, 13) # atrial flutter
    #H, a1, a2, a3, a4, Gt = (2.178, 0, 0, 0, -0.1, 21) # ventricular tachycardia
    #H, a1, a2, a3, a4, Gt = (2.178, 0.1, -0.02, -0.01, 0, 13) # ventricular flutter

    for i in range(len(t)):
        # x1.append(Gt*(x1[i]-x2[i]-C*x1[i]*x2[i]-x1[i]*x2[i]**2)*dt+x1[i])
        # x2.append(Gt*(H*x1[i]-3*x2[i]+C*x1[i]*x2[i]+x1[i]*x2[i]**2+b*(x4[i]-x2[i]))*dt+x2[i])
        # x3.append(Gt*(x3[i]-x4[i]-C*x3[i]*x4[i]-x3[i]*x4[i]**2)*dt+x3[i])
        # x4.append(Gt*(H*x3[i]-3*x4[i]+C*x3[i]*x4[i]+x3[i]*x4[i]**2+2*b*(x2[i]-x4[i]))*dt+x4[i])
        # ecg.append(a1*x1[i+1]+a2*x2[i+1]+a3*x3[i+1]+a4*x4[i+1])

        x1.append(Gt*(x1[-1]-x2[-1]-C*x1[-1]*x2[-1]-x1[-1]*x2[-1]**2)*dt+x1[-1])
        x2.append(Gt*(H*x1[-1]-3*x2[-1]+C*x1[-1]*x2[-1]+x1[-1]*x2[-1]**2+b*(x4[-1]-x2[-1]))*dt+x2[-1])
        x3.append(Gt*(x3[-1]-x4[-1]-C*x3[-1]*x4[-1]-x3[-1]*x4[-1]**2)*dt+x3[-1])
        x4.append(Gt*(H*x3[-1]-3*x4[-1]+C*x3[-1]*x4[-1]+x3[-1]*x4[-1]**2+2*b*(x2[-1]-x4[-1]))*dt+x4[-1])
        ecg.append(a1*x1[-1]+a2*x2[-1]+a3*x3[-1]+a4*x4[-1])

    return t, np.array(ecg)

# generate ECG signal
#t, ecg = ecg_generator(t_end=20)
#plt.plot(t, ecg)

# read ECG signal
#df = pd.read_csv('data_float.txt', names=['ecg']) # 32-bits float
df = pd.read_csv('data_char.txt', names=['ecg']) # 8-bits int
plt.plot(df['ecg'].iloc[0:10000])