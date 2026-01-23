#include <sstream>
#include <complex>
#include <vector>
#include <cmath>
#include <fstream>
#include <iostream>

static constexpr uint32_t TABLE_SIZE = 256;
static uint8_t sineTable[TABLE_SIZE];

static const uint16_t SAMPLE_RATE = 44100;

static volatile uint32_t phase = 0;

float k = 0.0f;
float k_step = 1.0f; 


void genTable(){

	// Build a 0..255 sine table
  for (uint32_t i = 0; i < TABLE_SIZE; i++) {
    float x = (2.0f * 3.14159265358979323846f * (float)i) / (float)TABLE_SIZE;
    float s = sinf(x);                 // -1..+1
    float u = (s * 0.5f) + 0.5f;       //  0..1
    int   y = (int)lroundf(u * 255.0f);//  0..255
    if (y < 0) y = 0;
    if (y > 255) y = 255;
    sineTable[i] = (uint8_t)y;

		// std::cout << y << std::endl;
	}
}

int main(int argc, char* argv[]){

	genTable();

	int input_freq;

	if (argc == 2){
		input_freq = atoi(argv[1]);
	} else {
		input_freq = 100;
	}

	k_step = (float)(input_freq * TABLE_SIZE) / (float)SAMPLE_RATE;

	// std::cout << "k_step " << k_step << std::endl;

	for (int i = 0; i < SAMPLE_RATE; i++) {
	
		int table_index = (int)std::floor(k) % TABLE_SIZE;
		std::cout << (int)sineTable[table_index] << std::endl;

		k += k_step;

	}
	
	return 0;
}
