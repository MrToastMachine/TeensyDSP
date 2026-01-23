#include <Arduino.h>

#include <sstream>
#include <complex>
#include <vector>
#include <cmath>
#include <fstream>
#include <iostream>


/* R2R DAC STUFF */

static constexpr uint8_t DAC_PIN_0 = 0; // LSB
static constexpr uint8_t DAC_PIN_1 = 1;
static constexpr uint8_t DAC_PIN_2 = 2;
static constexpr uint8_t DAC_PIN_3 = 3;
static constexpr uint8_t DAC_PIN_4 = 4;
static constexpr uint8_t DAC_PIN_5 = 5;
static constexpr uint8_t DAC_PIN_6 = 6;
static constexpr uint8_t DAC_PIN_7 = 7; // MSB



/* WAVETABLE GEN STUFF */

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

// Write an 8-bit value to the R2R ladder using pins 0..7 (LSB..MSB).
static inline void writeDAC(uint8_t v) {
  digitalWriteFast(DAC_PIN_0, (v >> 0) & 1);
  digitalWriteFast(DAC_PIN_1, (v >> 1) & 1);
  digitalWriteFast(DAC_PIN_2, (v >> 2) & 1);
  digitalWriteFast(DAC_PIN_3, (v >> 3) & 1);
  digitalWriteFast(DAC_PIN_4, (v >> 4) & 1);
  digitalWriteFast(DAC_PIN_5, (v >> 5) & 1);
  digitalWriteFast(DAC_PIN_6, (v >> 6) & 1);
  digitalWriteFast(DAC_PIN_7, (v >> 7) & 1);
}



void setup() {
  pinMode(DAC_PIN_0, OUTPUT);
  pinMode(DAC_PIN_1, OUTPUT);
  pinMode(DAC_PIN_2, OUTPUT);
  pinMode(DAC_PIN_3, OUTPUT);
  pinMode(DAC_PIN_4, OUTPUT);
  pinMode(DAC_PIN_5, OUTPUT);
  pinMode(DAC_PIN_6, OUTPUT);
  pinMode(DAC_PIN_7, OUTPUT);
	

	genTable();

	int input_freq;
	k_step = (float)(input_freq * TABLE_SIZE) / (float)SAMPLE_RATE;

}

void loop(){
	// check for flag flipped by interrupt
	// if flag > call R2R write
}
