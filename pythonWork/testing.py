# %%
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

filename = "data.csv"

data = pd.read_csv(filename, header=None).squeeze().to_numpy()




# %%
plt.figure()
plt.plot(data[2:, 0], data[2:, 1])
plt.xlabel('Column 0')
plt.ylabel('Column 1')
plt.show()