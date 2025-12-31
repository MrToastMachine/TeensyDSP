#include <vector>
#include <complex>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <print>


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

void loadWavFile(const std::string& filename, std::vector<std::complex<double>> dataOut) { 
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


		int numLines = 0;
    while(file.read(reinterpret_cast<char*>(mostRecentData.data()), 2)){
			numLines++;
      std::cout << "data read ok - " << mostRecentData[0] << mostRecentData[1] << std::endl;
    }

		std::cout << "Total Lines: " << numLines << std::endl;


		return;


    uint32_t samplesFor01Sec = header.sampleRate * 0.1;
    uint32_t bytesPerSample = header.bitsPerSample / 8;
    uint32_t totalSamples = samplesFor01Sec * header.numChannels;
    uint32_t bytesToRead = totalSamples * bytesPerSample;
    
    // Read audio data
    std::vector<uint8_t> audioData(bytesToRead);

    file.read(reinterpret_cast<char*>(audioData.data()), bytesToRead);
    
    if (file.gcount() < bytesToRead) {
        std::cerr << "Warning: Could only read " << file.gcount() << " bytes" << std::endl;
    }
   

    // Print audio data based on bit depth
    std::cout << "Audio Data (first 0.1 seconds):" << std::endl;


    if (header.bitsPerSample == 16) {
        // 16-bit signed PCM
        int16_t* samples = reinterpret_cast<int16_t*>(audioData.data());
        uint32_t numSamples = file.gcount() / 2;
        
        for (uint32_t i = 0; i < numSamples && i < 100; ++i) {
            std::cout << "Sample " << i << ": " << samples[i] << std::endl;
        }
        
        if (numSamples > 100) {
            std::cout << "... (showing first 100 samples only)" << std::endl;
        }
        
    } else {
        std::cerr << "Unsupported bit depth: " << header.bitsPerSample << std::endl;
    }
    
    file.close();
}

// The FFT function declared at 8:51
std::vector<std::complex<double>> fft(const std::vector<std::complex<double>>& samples) {
    // Get n, the number of samples (9:07)
    int n = static_cast<int>(samples.size());

    // Base case for recursion (9:12)
    if (n == 1) {
        return samples;
    }

    // Split samples into even and odd arrays (9:22 - 9:32)
    std::vector<std::complex<double>> even_samples(n / 2);
    std::vector<std::complex<double>> odd_samples(n / 2);

    for (int i = 0; i < n / 2; ++i) {
        even_samples[i] = samples[2 * i];
        odd_samples[i] = samples[2 * i + 1];
    }

    // Recursively run the above lines (9:36)
    std::vector<std::complex<double>> even_fft = fft(even_samples);
    std::vector<std::complex<double>> odd_fft = fft(odd_samples);

    // Combine the values at each level (9:42 - 10:02)
    std::vector<std::complex<double>> result(n);
    for (int k = 0; k < n / 2; ++k) {
        std::complex<double> t =
            std::polar(1.0, -2 * M_PI * k / n) * odd_fft[k];
        result[k] = even_fft[k] + t;
        result[k + n / 2] = even_fft[k] - t; // Takes advantage of symmetry
    }

    return result;
}

std::vector<std::complex<double>> loadCSVFile(std::string filename, int csv_len){
	std::ifstream file(filename, std::ios::binary);

	if (!file.is_open()) {
		std::cerr << "Error: could not open file " << filename << std::endl;
		// return None;
	}

	std::vector<std::complex<double>> returnVector(csv_len);
		
	std::string line;
	for (int i = 0; i < csv_len; i++) {
		std::getline(file, line);
		
		returnVector[i] = std::stof(line);

		//std::cout << line << std::endl;
	}

	return returnVector;


	std::vector<std::complex<double>> nullReturn = {0};
	return nullReturn;
}

void computeFFT(){
    std::vector<std::complex<double>> input_samples = {
      {0}
    }; // Example: samples from a sine wave as shown in video

    std::vector<std::complex<double>> output_freq_bins = fft(input_samples);

    // Print results (for testing)
    for (size_t i = 0; i < output_freq_bins.size(); ++i) {
        std::cout << "F[" << i << "] = " << output_freq_bins[i] << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // Example usage (not in video, but needed for a complete program)
    // std::vector<std::complex<double>> input_samples = {
    //     {0, 0}, {1, 0}, {0, 0}, {-1, 0}
    // }; // Example: samples from a sine wave as shown in video

		if (argc < 2){
			std::cout << "not enough arguments" << std::endl;
		}

		std::string filename = argv[1];

		std::vector<std::complex<double>> wavData;

		loadWavFile(filename, wavData);

		return 0;
}

