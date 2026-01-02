#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
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

void printWavMetaData(const std::string& filename, bool printFirst100Samples=false){

    std::ifstream file(filename, std::ios::binary);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }
    
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
}

void parseWAV(const std::string& filename, bool printFirst100Samples=false) {
    std::ifstream file(filename, std::ios::binary);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }
    
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
    
    // Calculate samples for 0.1 seconds
    uint32_t samplesFor01Sec = header.sampleRate * 0.1;
    uint32_t bytesPerSample = header.bitsPerSample / 8;
    uint32_t totalSamples = samplesFor01Sec * header.numChannels;
    uint32_t bytesToRead = totalSamples * bytesPerSample;
    
    std::cout << "Reading first 0.1 seconds (" << samplesFor01Sec << " samples per channel)" << std::endl;
    std::cout << "Total bytes to read: " << bytesToRead << std::endl;
    std::cout << std::endl;
    
    // Read audio data
    std::vector<uint8_t> audioData(bytesToRead);
    file.read(reinterpret_cast<char*>(audioData.data()), bytesToRead);
    
    if (file.gcount() < bytesToRead) {
        std::cerr << "Warning: Could only read " << file.gcount() << " bytes" << std::endl;
    }
   
		if (!printFirst100Samples){
			std::print("Not printing firsto 100 samples...\n");
			return;
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
        
    } else if (header.bitsPerSample == 8) {
        // 8-bit unsigned PCM
        for (uint32_t i = 0; i < audioData.size() && i < 100; ++i) {
            std::cout << "Sample " << i << ": " << static_cast<int>(audioData[i]) << std::endl;
        }
        
        if (audioData.size() > 100) {
            std::cout << "... (showing first 100 samples only)" << std::endl;
        }
        
    } else if (header.bitsPerSample == 32) {
        // 32-bit PCM (could be integer or float)
        if (header.audioFormat == 1) {
            // 32-bit signed integer PCM
            int32_t* samples = reinterpret_cast<int32_t*>(audioData.data());
            uint32_t numSamples = file.gcount() / 4;
            
            for (uint32_t i = 0; i < numSamples && i < 100; ++i) {
                std::cout << "Sample " << i << ": " << samples[i] << std::endl;
            }
            
            if (numSamples > 100) {
                std::cout << "... (showing first 100 samples only)" << std::endl;
            }
        } else if (header.audioFormat == 3) {
            // 32-bit float PCM
            float* samples = reinterpret_cast<float*>(audioData.data());
            uint32_t numSamples = file.gcount() / 4;
            
            for (uint32_t i = 0; i < numSamples && i < 100; ++i) {
                std::cout  << samples[i] << std::endl;
            }
            
            if (numSamples > 100) {
                std::cout << "... (showing first 100 samples only)" << std::endl;
            }
        } else {
            std::cerr << "Unsupported 32-bit audio format: " << header.audioFormat << std::endl;
        }
        
    } else {
        std::cerr << "Unsupported bit depth: " << header.bitsPerSample << std::endl;
    }
    
    file.close();
}

int main(int argc, char *argv[]){
	if (argc < 2){
		std::printf("Expected -> main <wav_file>\n");
		return 1;
	}
	

	bool printLines = false;

	if (argc == 3){
		printLines = argv[2];
	}

	//std::string filename = "moosic.wav";
	std::string filename = argv[1];
	parseWAV(filename, printLines);
	return 0;
}
