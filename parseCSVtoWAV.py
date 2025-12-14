import csv
import sys
import wave
import struct

# Default sample rate (Hz) – change if you used something else
DEFAULT_SAMPLE_RATE = 48000

def read_adc_csv(path):
    """
    Reads a CSV file with either:
      index,sample
    or just
      sample

    Returns a list of ADC values as floats.
    """
    samples = []
    with open(path, "r", newline="") as f:
        reader = csv.reader(f)
        for row in reader:
            if not row:
                continue

            # Try to interpret last column as the sample value
            try:
                value = float(row[-1])
            except ValueError:
                # Skip header or non-numeric lines
                continue

            samples.append(value)
    return samples

def adc_to_int16(samples, adc_bits=12):
    """
    Convert ADC samples (0..2^adc_bits-1) to signed int16 audio.

    Assumes mid-scale (e.g. 2048 for 12-bit) is "zero" and scales around that.
    """
    max_adc = (1 << adc_bits) - 1  # e.g. 4095
    mid = max_adc / 2.0            # e.g. 2047.5

    int16_samples = []
    for v in samples:
        # Center around 0
        centered = v - mid  # now roughly -2048..+2048

        # Normalize to -1..+1 (approx)
        norm = centered / mid

        # Scale to int16 range
        s = int(norm * 32767)

        # Clip just in case
        if s > 32767:
            s = 32767
        if s < -32768:
            s = -32768

        int16_samples.append(s)

    return int16_samples

def write_wav(path, samples_int16, sample_rate=DEFAULT_SAMPLE_RATE):
    """
    Write mono 16-bit PCM WAV.
    """
    with wave.open(path, "w") as wf:
        n_channels = 1
        sampwidth = 2  # bytes (16-bit)
        n_frames = len(samples_int16)
        comptype = "NONE"
        compname = "not compressed"

        wf.setparams((n_channels, sampwidth, sample_rate, n_frames, comptype, compname))

        # Pack samples as little-endian 16-bit
        frames = b"".join(struct.pack("<h", s) for s in samples_int16)
        wf.writeframes(frames)

def main():
    if len(sys.argv) < 3:
        print("Usage: python csv_to_wav.py input.csv output.wav [sample_rate]")
        sys.exit(1)

    in_path = sys.argv[1]
    out_path = sys.argv[2]

    if len(sys.argv) >= 4:
        sample_rate = int(sys.argv[3])
    else:
        sample_rate = DEFAULT_SAMPLE_RATE

    print(f"Reading CSV: {in_path}")
    adc_samples = read_adc_csv(in_path)
    print(f"Read {len(adc_samples)} samples")

    print("Converting ADC values to 16-bit audio...")
    int16_samples = adc_to_int16(adc_samples, adc_bits=12)

    print(f"Writing WAV: {out_path} at {sample_rate} Hz")
    write_wav(out_path, int16_samples, sample_rate)

    print("Done.")

if __name__ == "__main__":
    main()

