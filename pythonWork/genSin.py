import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

t = np.linspace(0,1,48000)

f1 = 60
f2 = f1 * 3
f3 = 7 * f1

y1 = 10000 * np.sin(t * 2 * np.pi * f1)
y2 = 10000 * np.sin(t * 2 * np.pi * f2)
y3 = 10000 * np.sin(t * 2 * np.pi * f3)

signal = y1 + y2 + y3

signal = np.round(signal).astype(np.int16)
np.savetxt("out.csv", signal, fmt="%d", delimiter=",")



plt.plot(signal)
plt.show()
