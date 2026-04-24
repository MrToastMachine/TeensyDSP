// dspHelpers.h
// Contains functions used for dsp and display stuff

#include <math.h>
#include <stdio.h>
#include <complex>
#include <string>

#define NUM_FREQS 49

// This starts at C1 (octave 1) and goes to A4 -----> ONLY A:440Hz TUNING
const float allPossibleFreqs[NUM_FREQS] = {32.7 ,  34.65,  36.71,  38.89,  41.2 ,
        43.65,  46.25,  49.  ,  51.91,  55.  ,  58.27,  61.74,  65.41,
        69.3 ,  73.42,  77.78,  82.41,  87.31,  92.5 ,  98.  , 103.83,
       110.0  , 116.54, 123.47, 130.81, 138.59, 146.83, 155.56, 164.81,
       174.61, 185.  , 196.  , 207.65, 220.  , 233.08, 246.94, 261.63,
       277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392.  , 415.3 ,
       440.0  };

const std::string NOTES[12] = {
	"C",
	"C#",
	"D",
	"D#",
	"E",
	"F",
	"F#",
	"G",
	"G#",
	"A",
	"A#",
	"B"
};

int mapFloatToInt(float f_low, float f_high, int d_low, int d_high, float input);
int findClosestIndex(const float *arr, uint8_t arrLen, float target);
void getNearestNoteAndPrint(float note);
int getXValForLED(float note, std::string &current_closest_note);
