import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

t = np.linspace(0,1,48000)

y1 = 10000 * np.sin(t * 2 * np.pi * 5)
y2 = 10000 * np.sin(t * 2 * np.pi * 8)
y3 = 10000 * np.sin(t * 2 * np.pi * 11)

signal = y1 + y2 + y3

signal = np.round(signal).astype(np.int32)
np.savetxt("out.csv", signal, fmt="%d", delimiter=",")



plt.plot(signal)
plt.show()
