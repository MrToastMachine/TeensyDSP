import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy import signal
import pandas as pd
import argparse
import os


def convertCSV_to_Wav(csv_file, sampleRate=None):

    if not sampleRate:
        sampleRate = 48000
    wav_file = csv_file.split('.')[0] + ".wav"

    file = pd.read_csv(csv_file, header=None, dtype=np.int16)
    audio_data = file.values.flatten()

    wavfile.write(wav_file, sampleRate, audio_data)


  

def main(testFile=None):

    if testFile:
        convertCSV_to_Wav(testFile)
        return

    parser = argparse.ArgumentParser(description='Convert CSV to WAV file')
    parser.add_argument('csv_file', help='Path to the CSV file')
    parser.add_argument('--sample_rate', help='Sample rate of CSV file')
    
    args = parser.parse_args()

    convertCSV_to_Wav(args.csv_file, args.sample_rate)

    


if __name__ == "__main__":
    # Example usage when running directly
    import sys
    
    if len(sys.argv) < 2:
        sys.exit(main('pythonWork/out.csv'))

        # print("Usage: python spectrogram_plotter.py <csv_file> [-o output_file]")
        # print("Example: python spectrogram_plotter.py audio.csv -o spectrogram.png")
        # sys.exit(1)
    
    sys.exit(main())
