#include <Arduino.h>

// Capacitive Touch Libs
#include <Wire.h>
#include "Adafruit_MPR121.h"


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

static const float SAMPLE_RATE = 44100.0;
static const float PERIOD_US = 1000000.0/SAMPLE_RATE;

static volatile uint32_t phase = 0;

float k = 0.0f;
float k_step = 1.0f; 

static IntervalTimer sampleTimer;
static bool sample_flag = false;

static bool note_pressed = true;

int newest_sample = 0;

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

void calculateNextSample(){
	int table_index = (int)std::floor(k) % TABLE_SIZE;

	// TODO: Replace this line with general reference to 'active_table'
	// instead of directly calling sineTable
	newest_sample = sineTable[table_index];

	k += k_step;
}

void onSampleTick(){
	sample_flag = note_pressed;
}

void testTiming(){

  int start_time, end_time;

  start_time = micros();
  writeDAC(newest_sample);
  calculateNextSample();
  sample_flag = false;
  end_time = micros();


  Serial.print("Time taken single sample: ");
  Serial.println(end_time - start_time);
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

	k_step = (float)(440 * TABLE_SIZE) / (float)SAMPLE_RATE;

  Serial.begin(115200);
  while (!Serial){
    delay(100);
  }

	sampleTimer.begin(onSampleTick, PERIOD_US);

  // for (int i = 0; i < 100; i++)
  // {
  //   testTiming();
  // }
  
}

void loop(){



	if (sample_flag){
		writeDAC(newest_sample);
		calculateNextSample();
    sample_flag = false;
	}
}






















