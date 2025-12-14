import numpy as np
import matplotlib.pyplot as plt

SAMPLE_RATE = 48000
nSecs = 1
freq = 100

t = np.linspace(0,nSecs, SAMPLE_RATE*nSecs)

y = np.sin(t * 2 * np.pi * freq)

plt.plot(t,y)
plt.show()

nVals = len(y)
with open('sineWave.csv', 'w') as file:
  for i, val in enumerate(y):
    file.write(f"{val}")
    if i != nVals - 1:
      file.write(",\n")
    
print("Wrote to file")
