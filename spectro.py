# TODO: generalize to accept wav or csv file


import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy import signal
import argparse
import os

def plot_spectrogram(wav_file, output_file=None, fmax=None):
    """
    Read a WAV file and plot its spectrogram
    
    Parameters:
    wav_file (str): Path to the WAV file
    output_file (str): Optional path to save the plot
    fmax (float): Maximum frequency to display (Hz)
    """
    
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
#    if audio_data.dtype == np.int16:
#        audio_data = audio_data.astype(np.float32) / 32768.0
#    elif audio_data.dtype == np.int32:
#        audio_data = audio_data.astype(np.float32) / 2147483648.0
    
    # Create the spectrogram
    frequencies, times, Sxx = signal.spectrogram(
        audio_data, 
        sample_rate,
        window='hann',
        nperseg=1024,
        noverlap=512,
        nfft=1024
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
    plt.title(f'Spectrogram of {os.path.basename(wav_file)}')
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
    
    # Save or show the plot
    if output_file:
        plt.savefig(output_file, dpi=300, bbox_inches='tight')
        print(f"Spectrogram saved to: {output_file}")
    else:
        plt.show()

def main():
    parser = argparse.ArgumentParser(description='Plot spectrogram of a WAV file')
    parser.add_argument('wav_file', help='Path to the WAV file')
    parser.add_argument('-o', '--output', help='Output file path for saving the plot')
    parser.add_argument('--fmax', type=float, help='Maximum frequency to display (Hz)')
    
    args = parser.parse_args()
    
    try:
        plot_spectrogram(args.wav_file, args.output, args.fmax)
    except Exception as e:
        print(f"Error: {e}")
        return 1
    
    return 0

if __name__ == "__main__":
    # Example usage when running directly
    import sys
    
    if len(sys.argv) < 2:
        print("Usage: python spectrogram_plotter.py <wav_file> [-o output_file]")
        print("Example: python spectrogram_plotter.py audio.wav -o spectrogram.png")
        sys.exit(1)
    
    sys.exit(main())
