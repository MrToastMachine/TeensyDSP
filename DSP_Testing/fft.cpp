#include <vector>
#include <complex>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>

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

		std::vector<std::complex<double>> csvData;

		loadCSVFile(filename, 48000);

		return 0;
}

