#include <sstream>
#include <complex>
#include <vector>
#include <cmath>
#include <fstream>
#include <iostream>

static constexpr uint32_t TABLE_SIZE = 256;
static uint8_t sineTable[TABLE_SIZE];

static volatile uint32_t phase = 0;

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

		std::cout << y << std::endl;
	}
}

int main(int argc, char* argv[]){

	int numIterations;

	if (argc == 2){
		numIterations = atoi(argv[1]);
	} else {
		numIterations = 3;
	}

	for (int i = 0; i < numIterations; i++) {
	
		genTable();

	}
	return 0;
}
