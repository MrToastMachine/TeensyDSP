import sys
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import argparse
import os


if len(sys.argv) < 2:
  print("Youre using this wrong")
  sys.exit(1)


parser = argparse.ArgumentParser(description="Plot contents of csv")
parser.add_argument('file', help="Path to csv file")

args = parser.parse_args()

if args.file == None:
    print("No file given")
    sys.exit()

csv_file = args.file
df = pd.read_csv(csv_file)

plt.plot(df.iloc[:,0], df.iloc[:,1])
# plt.xlim([0,1000])
plt.show()


"""
plt.plot(arr[:,1])
plt.show()
"""
