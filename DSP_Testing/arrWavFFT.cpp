#include <vector>
#include <complex>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <print>
#include <stdlib.h>

struct WAVHeader {
	char riff[4];           // "RIFF"
	uint32_t fileSize;      // File size - 8
	char wave[4];           // "WAVE"
	char fmt[4];            // "fmt "
	uint32_t fmtSize;       // Format chunk size
	uint16_t audioFormat;   // Audio format (1 = PCM)
	uint16_t numChannels;   // Number of channels
	uint32_t sampleRate;    // Sample rate
	uint32_t byteRate;      // Byte rate
	uint16_t blockAlign;    // Block alignment
	uint16_t bitsPerSample; // Bits per sample
	char data[4];           // "data"
	uint32_t dataSize;      // Data chunk size
};


// FFT Specific

const int NUM_SAMPLES = 8192;

using Complex = std::complex<double>;
const Complex* sampleBank[NUM_SAMPLES] = {0};

void loadWavFile(const std::string& filename, std::vector<Complex>& data_out, int num_datapoints = 0) {
	std::ifstream file(filename, std::ios::binary);
	
	if (!file.is_open()) {
			std::cerr << "Error: Could not open file " << filename << std::endl;
			return;
	}

	std::cout << "Opening file " << filename << std::endl;
	
	WAVHeader header;
	file.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));
	
	// Verify it's a valid WAV file
	if (strncmp(header.riff, "RIFF", 4) != 0 || 
			strncmp(header.wave, "WAVE", 4) != 0) {
			std::cerr << "Error: Not a valid WAV file" << std::endl;
			return;
	}
	
	// Print WAV file info
	std::cout << "WAV File Information:" << std::endl;
	std::cout << "Sample Rate: " << header.sampleRate << " Hz" << std::endl;
	std::cout << "Channels: " << header.numChannels << std::endl;
	std::cout << "Bits per Sample: " << header.bitsPerSample << std::endl;
	std::cout << "Audio Format: " << header.audioFormat << std::endl;
	std::cout << "Data Size: " << header.dataSize << " bytes" << std::endl;
	std::cout << std::endl;
 
	std::vector<uint16_t> alldata;
	std::vector<uint8_t> mostRecentData(2);

	int16_t newest_val;
	
	int num_lines = 0;
	while(file.read(reinterpret_cast<char*>(&newest_val), 2)){
	//while(file.read(atoi(newest_val), 2)){
		num_lines++;
		
		if (num_lines < 10 && false){
			std::cout << newest_val << std::endl;
		}
	
		alldata.push_back(newest_val);

		// newest_val is a 16-bit signed integer, convert to double and store as complex number (imaginary part = 0)
		Complex c = Complex(static_cast<double>(newest_val));

		data_out.push_back(c);
		
		sampleBank[num_lines-1] = c;


		if (num_datapoints == num_lines){
			break;
		}
	}

	std::cout << "Total Lines: " << num_lines << std::endl;

	file.close();
}

std::vector<Complex> fft(const std::vector<Complex>& samples) {
    // Get n, the number of samples 
    int n = static_cast<int>(samples.size());

    // Base case for recursion 
    if (n == 1) {
        return samples;
    }

    // Split samples into even and odd arrays
    std::vector<Complex> even_samples(n / 2);
    std::vector<Complex> odd_samples(n / 2);

    for (int i = 0; i < n / 2; ++i) {
        even_samples[i] = samples[2 * i];
        odd_samples[i] = samples[2 * i + 1];
    }

    // Recursively run the above lines
    std::vector<Complex> even_fft = fft(even_samples);
    std::vector<Complex> odd_fft = fft(odd_samples);

    // Combine the values at each level
    std::vector<Complex> result(n);
    for (int k = 0; k < n / 2; ++k) {
        Complex t =
            std::polar(1.0, -2 * M_PI * k / n) * odd_fft[k];
        result[k] = even_fft[k] + t;
        result[k + n / 2] = even_fft[k] - t; // Takes advantage of symmetry
    }

    return result;
}

void fft_arr(const Complex* samples, Complex* result, int n) {
    // Base case
    if (n == 1) {
        result[0] = samples[0];
        return;
    }

    // Split into even and odd
    Complex* even_samples = new Complex[n / 2];
    Complex* odd_samples  = new Complex[n / 2];

    for (int i = 0; i < n / 2; ++i) {
        even_samples[i] = samples[2 * i];
        odd_samples[i]  = samples[2 * i + 1];
    }

    // Recurse into output buffers
    Complex* even_fft = new Complex[n / 2];
    Complex* odd_fft  = new Complex[n / 2];

    fft_arr(even_samples, even_fft, n / 2);
    fft_arr(odd_samples,  odd_fft,  n / 2);

    // Combine
    for (int k = 0; k < n / 2; ++k) {
        Complex t = std::polar(1.0, -2.0 * M_PI * k / n) * odd_fft[k];
        result[k]         = even_fft[k] + t;
        result[k + n / 2] = even_fft[k] - t;
    }

    delete[] even_samples;
    delete[] odd_samples;
    delete[] even_fft;
    delete[] odd_fft;
}

void computeFFT(const std::string& filename, int num_datapoints = 0){

	std::vector<Complex> wavData;

	loadWavFile(filename, wavData, num_datapoints);


	// wavData here is the equivalent of one full block ready to analyse


	std::vector<Complex> output_freq_bins = fft(wavData);

	// Print results (for testing)
	for (size_t i = 0; i < output_freq_bins.size(); ++i) {
			// std::cout << "F[" << i << "] = " << output_freq_bins[i] << std::endl;
			std::cout << i << ", " << std::abs(output_freq_bins[i]) << std::endl;
	}
		
}

int main(int argc, char* argv[]) {

	int nPoints = 0;

	if (argc < 2){
		std::cout << "not enough arguments" << std::endl;
		return 1;
	}

	if (std::atoi(argv[2])){
		nPoints = std::atoi(argv[2]);
	}
	
	std::string filename = argv[1];

	computeFFT(filename, 16384);
	
	return 0;
}

