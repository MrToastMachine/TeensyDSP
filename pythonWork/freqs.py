import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy import signal
import argparse
import os
import sys


DEFAULT_FILE_PATH = sys.argv[0][::-1].partition('/')[2][::-1] + "/out.wav"

print(DEFAULT_FILE_PATH)

def plot_freq_content(wav_file, output_file=None, fmax=None):
    
    # Check if file exists
    if not os.path.exists(wav_file):
        raise FileNotFoundError(f"WAV file not found: {wav_file}")
    
    # Read the WAV file
    try:
        sample_rate, audio_data = wavfile.read(wav_file)
        print(f"Sample rate: {sample_rate} Hz")
        print(f"Audio shape: {audio_data.shape}")
        print(f"Duration: {len(audio_data) / sample_rate:.2f} seconds")
    except Exception as e:
        raise Exception(f"Error reading WAV file: {e}")
    
    # Handle stereo audio by taking the first channel
    if len(audio_data.shape) > 1:
        audio_data = audio_data[:, 0]
        print("Stereo audio detected, using first channel")
    
    # Convert to float if integer
    if audio_data.dtype == np.int16:
        audio_data = audio_data.astype(np.float32) / 32768.0
    elif audio_data.dtype == np.int32:
        audio_data = audio_data.astype(np.float32) / 2147483648.0
   
    f, h = signal.welch(audio_data, fs=sample_rate, nperseg=4096, window='blackman')
        # Convert to dB scale
    Sxx_db = 10 * np.log10(h + 1e-10)  # Add small value to avoid log(0)
    
    # Create the plot
    plt.figure(figsize=(12, 8))
    
    # Plot spectrogram
    plt.subplot(2, 1, 1)
    plt.plot(f, Sxx_db)
    plt.ylabel('Power')
    plt.xlabel('Frequency (Hz)')
    plt.title(f'Freq content of {os.path.basename(wav_file)}')
    
    # Plot waveform for reference
    plt.subplot(2, 1, 2)
    time_axis = np.linspace(0, len(audio_data) / sample_rate, len(audio_data))
    plt.plot(time_axis, audio_data)
    plt.xlabel('Time (s)')
    plt.ylabel('Amplitude')
    plt.title('Waveform')
    plt.grid(True, alpha=0.3)
    
    plt.tight_layout()
    
    # Save or show the plot
    if output_file:
        plt.savefig(output_file, dpi=300, bbox_inches='tight')
        print(f"Spectrogram saved to: {output_file}")
    else:
        plt.show()

def main(testFile=None):

    if testFile:
        plot_freq_content(testFile)
        return

    parser = argparse.ArgumentParser(description='Plot spectrogram of a WAV file')
    parser.add_argument('wav_file', help='Path to the WAV file')
    parser.add_argument('-o', '--output', help='Output file path for saving the plot')
    parser.add_argument('--fmax', type=float, help='Maximum frequency to display (Hz)')
    
    args = parser.parse_args()
    
    try:
        plot_freq_content(args.wav_file, args.output, args.fmax)
    except Exception as e:
        print(f"Error: {e}")
        return 1
    
    return 0

if __name__ == "__main__":
    
    if len(sys.argv) < 2:
        print(f"Running file '{sys.argv[0]}")

        print("Usage: python spectrogram_plotter.py <wav_file> [-o output_file]")
        print("Example: python spectrogram_plotter.py audio.wav -o spectrogram.png")

        print("Defaulting to file '{DEFAULT_FILE_PATH}'")

        sys.exit(main(DEFAULT_FILE_PATH))
    
    sys.exit(main())
