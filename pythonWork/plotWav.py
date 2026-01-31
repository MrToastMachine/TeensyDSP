import sys
import argparse
import numpy as np
import pandas as pd
from scipy.io import wavfile
from scipy import signal
import matplotlib.pyplot as plt

def main(file_path):
 
  sample_rate, audio_data = wavfile.read(file_path)

  freqs, psd = signal.welch(audio_data, sample_rate, nperseg=16384)
  
  plt.figure(figsize=(10,6))
  plt.subplot(211)
  plt.plot(audio_data)

  plt.subplot(212)
  plt.plot(freqs, psd)
  plt.xlim([0,1000])

  plt.show()


  return 0

if __name__ == "__main__":
  
  if len(sys.argv) < 2:
    print(f"Running file '{sys.argv[0]}")

    print("Usage: python spectrogram_plotter.py <wav_file> [-o output_file]")
    print("Example: python spectrogram_plotter.py audio.wav -o spectrogram.png")

    print("Defaulting to file '{DEFAULT_FILE_PATH}'")

    sys.exit(main(DEFAULT_FILE_PATH))
  
  parser = argparse.ArgumentParser(description='Plot spectrogram of a WAV file')
  parser.add_argument('wav_file', help='Path to the WAV file')
  
  args = parser.parse_args()
  
  sys.exit(main(args.wav_file))
