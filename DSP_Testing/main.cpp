#include <sstream>
#include <complex>
#include <vector>
#include <cmath>
#include <fstream>
#include <iostream>

const uint32_t SAMPLE_RATE   = 48000;            // Hz
const uint32_t DURATION_SEC  = 1;                // seconds
const uint32_t NUM_SAMPLES   = SAMPLE_RATE * DURATION_SEC;
const uint32_t SAMPLE_PERIOD_US = 1000000UL / SAMPLE_RATE; // ~20 us

uint16_t samples[NUM_SAMPLES];

// ---------- FFT implementation (radix-2, iterative) ----------

void fft(std::vector<std::complex<double>>& a) {
    const std::size_t n = a.size();
    if (n == 0) return;

    // Bit-reversal permutation
    std::size_t j = 0;
    for (std::size_t i = 1; i < n; ++i) {
        std::size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            std::swap(a[i], a[j]);
        }
    }

    // Cooley–Tukey
    for (std::size_t len = 2; len <= n; len <<= 1) {
        double ang = -2.0 * M_PI / static_cast<double>(len);
        std::complex<double> wlen(std::cos(ang), std::sin(ang));
        for (std::size_t i = 0; i < n; i += len) {
            std::complex<double> w(1.0, 0.0);
            for (std::size_t j = 0; j < len / 2; ++j) {
                std::complex<double> u = a[i + j];
                std::complex<double> v = a[i + j + len / 2] * w;
                a[i + j]             = u + v;
                a[i + j + len / 2]   = u - v;
                w *= wlen;
            }
        }
    }
}

// Next power of two ≥ n
std::size_t nextPow2(std::size_t n) {
    std::size_t p = 1;
    while (p < n) p <<= 1;
    return p;
}


// ... all your includes and FFT code unchanged ...

int main(int argc, char* argv[]){
	
    if (argc < 2){
        std::printf("Expected main <csv_file>");
        return 1;
    }

    std::string filename = argv[1];
	
    // Read CSV into known length array
    std::ifstream infile(filename, std::ios::binary);
	
    if (!infile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return 1;
    }

    std::string line;
    for(int i = 0; i < NUM_SAMPLES; i++){
        std::getline(infile, line);

        std::stringstream parsed(line);
        std::string item;		

        std::getline(parsed, item, ',');	
        std::getline(parsed, item, ',');	
        samples[i] = std::stoi(item);
    }

    // ---- 2) Zero-pad to power-of-two FFT length ----
    std::size_t N = nextPow2(NUM_SAMPLES);
    std::vector<std::complex<double>> X(N);

    for (std::size_t i = 0; i < NUM_SAMPLES; ++i) {
        X[i] = std::complex<double>(samples[i], 0.0);
    }
    for (std::size_t i = NUM_SAMPLES; i < N; ++i) {
        X[i] = std::complex<double>(0.0, 0.0);
    }

    // ---- 3) Run FFT ----
    fft(X);

    // ---- 4) Write frequency vs power to CSV ----
    std::ofstream outfile("fft_spectrum.csv");
    if (!outfile) {
        std::cerr << "Failed to open output CSV file\n";
        return 1;
    }

    outfile << "frequency_hz,power\n";

    std::size_t nyquistBin = N / 2;
    for (std::size_t k = 0; k <= nyquistBin; ++k) {
        double freq = (static_cast<double>(k) * SAMPLE_RATE) / static_cast<double>(N);
        double power = std::norm(X[k]);

        outfile << freq << "," << power << "\n";
    }

    outfile.close();
    std::cout << "Wrote spectrum to fft_spectrum.csv\n";

    return 0;
}
