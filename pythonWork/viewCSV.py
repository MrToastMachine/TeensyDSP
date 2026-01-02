import numpy as np
import matplotlib.pyplot as plt
import argparse
import os

def main():
    parser = argparse.ArgumentParser(description="Plot contents of csv")
    parser.add_argument('file', help="Path to csv file")

    args = parser.parse_args()

    if args.file == None:
        print("No file given")
        return

    arr = np.loadtxt(args.file, delimiter=",", dtype=str)


    plt.plot(arr[:,1])
    plt.show()


if __name__=="__main__":
    import sys

    if len(sys.argv) < 2:
        print("Youre using this wrong")
        sys.exit(1)

    sys.exit(main())

