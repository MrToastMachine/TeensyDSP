import numpy as np
import matplotlib.pyplot as plt

arr = np.loadtxt("fftOut.csv", delimiter=",", dtype=str)


plt.plot(arr)
plt.show()
