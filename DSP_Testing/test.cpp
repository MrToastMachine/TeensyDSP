#include <iostream>
#include <math.h>
#include <stdio.h>
#include <complex>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 96

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


int findClosestIndex(const float *arr, uint8_t arrLen, float target){
    float res = arr[0];
    int lo = 0, hi = arrLen;

		int closest_index = -1;

    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;

        // Update res if mid is closer to target
        if (abs(arr[mid] - target) < abs(res - target)) {
            res = arr[mid];
						closest_index = mid;
            // In case of a tie, prefer larger value
        }
        else if (abs(arr[mid] - target) == abs(res - target)) {
            res = fmax(res, arr[mid]);
        }

        if (arr[mid] == target) {
            // return arr[mid];
            return mid;
        }
        else if (arr[mid] < target) {
            lo = mid + 1;
        }
        else {
            hi = mid - 1;
        }
    }

		return closest_index;
    // return res;
}

void getNearestNoteAndPrint(float note){
	int index = findClosestIndex(allPossibleFreqs, NUM_FREQS, note);
	float closest_note_actual = allPossibleFreqs[index];
	int note_index = index % 12;

	int note_octave = index / 12 + 1;

	printf("Closest index in freq bank -> %d\n", index);

	printf("Closest Frequency -> %f\n", closest_note_actual);

	printf("The closest note is %s%d\n", NOTES[note_index].c_str(), note_octave);

	// The next thing to do is check distance to nearest notes
	//
	// Plot nearest note in center
	// halfway to one note lower on left
	// halfway to one note higher on right
	// map this range to screen (arbitrary)
	
	printf("\n");
		
	int higher_note_index = std::min(index + 1, NUM_FREQS - 1);
	int lower_note_index = std::max(index - 1, 0);
	
	printf("One note higher >> %f\n", allPossibleFreqs[higher_note_index]);
	printf("One note lower >> %f\n", allPossibleFreqs[lower_note_index]);


	float halfway_to_higher_note = (allPossibleFreqs[higher_note_index] - closest_note_actual) / 2;
	float halfway_to_lower_note = (closest_note_actual - allPossibleFreqs[lower_note_index]) / 2;

	printf("Distance to halfway to up: %f\n", halfway_to_higher_note);
	printf("Distance to halfway to down: %f\n", halfway_to_lower_note);

	//
}

int main(int argc, char* argv[]){


	if (argc < 2){
		printf("not enough arguments\n");
		return 1;
	}

	float test = std::atof(argv[1]);
	printf("Arg given -> %f\n", test);
	getNearestNoteAndPrint(test);


	return 0;	
}

