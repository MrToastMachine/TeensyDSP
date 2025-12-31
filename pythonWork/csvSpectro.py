# TODO: generalize to accept wav or csv file

import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy import signal
import pandas as pd
import argparse
import os


def plot_spectrogram(csv_file, fmax=None, fs=48000):    

    sample_rate = fs

    file = pd.read_csv(csv_file, header=None)
    audio_data = file.values.flatten()

    # with open(csv_file, 'r') as file:
    #     arr = file.read().split(',')
    #     audio_data = np.array([float(val) for val in arr])
  

    # Create the spectrogram
    frequencies, times, Sxx = signal.spectrogram(
        audio_data, 
        sample_rate,
        window='hann',
        nperseg=1024,
        noverlap=512,
        nfft=4096
    )
    
    # Convert to dB scale
    Sxx_db = 10 * np.log10(Sxx + 1e-10)  # Add small value to avoid log(0)
    
    # Create the plot
    plt.figure(figsize=(12, 8))
    
    # Plot spectrogram
    plt.subplot(2, 1, 1)
    plt.pcolormesh(times, frequencies, Sxx_db, shading='gouraud', cmap='viridis')
    plt.ylabel('Frequency (Hz)')
    plt.xlabel('Time (s)')
    # plt.title(f'Spectrogram of {os.path.basename(csv_file)}')
    plt.colorbar(label='Power (dB)')
    # Set frequency limits if specified
    if fmax is not None:
        plt.ylim(0, fmax)
    
    # Plot waveform for reference
    plt.subplot(2, 1, 2)
    time_axis = np.linspace(0, len(audio_data) / sample_rate, len(audio_data))
    plt.plot(time_axis, audio_data)
    plt.xlabel('Time (s)')
    plt.ylabel('Amplitude')
    plt.title('Waveform')
    plt.grid(True, alpha=0.3)
    
    plt.tight_layout()
    
    plt.show()

def main(testFile=None):

    if testFile:
        try:
            plot_spectrogram(testFile, fmax=300)
        except Exception as e:
            print("error in main")
            print(f"Error: {e}")
            return 1
        return 0
    

    parser = argparse.ArgumentParser(description='Plot spectrogram of a WAV file')
    parser.add_argument('csv_file', help='Path to the WAV file')
    parser.add_argument('--fmax', type=float, help='Maximum frequency to display (Hz)')
    
    args = parser.parse_args()
    
    try:
        # plot_spectrogram(args.csv_file)
        plot_spectrogram(args.csv_file, args.fmax)
    except Exception as e:
        print("Error goop")
        # print(f"Error: {e}")
        return
    
    return 0

if __name__ == "__main__":
    # Example usage when running directly
    import sys
    
    if len(sys.argv) < 2:
        sys.exit(main('pythonWork/out.csv'))

        # print("Usage: python spectrogram_plotter.py <csv_file> [-o output_file]")
        # print("Example: python spectrogram_plotter.py audio.csv -o spectrogram.png")
        # sys.exit(1)
    
    sys.exit(main())
