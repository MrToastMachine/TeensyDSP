// dspHelper.cpp
//

#include "main.h"
#include "dspHelpers.h"

int mapFloatToInt(float f_low, float f_high, int d_low, int d_high, float input){
	// pos of input in range f_low to f_high on a scale of 0 to 1
	float progress = (input - f_low) / (f_high - f_low);

	int int_full_scale = d_high - d_low;
	int int_progress = round((float)int_full_scale * progress) + d_low;

	return int_progress;
}

int findClosestIndex(const float *arr, uint8_t arrLen, float target){
    float res = arr[0];
    int lo = 0, hi = arrLen-1;

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
		
	int higher_note_index = std::min(index + 1, NUM_FREQS - 1);
	int lower_note_index = std::max(index - 1, 0);
	

	float halfway_to_higher_note = (allPossibleFreqs[higher_note_index] - closest_note_actual) / 2;
	float halfway_to_lower_note = (closest_note_actual - allPossibleFreqs[lower_note_index]) / 2;

	int x_pos_screen = mapFloatToInt(closest_note_actual - halfway_to_lower_note, closest_note_actual + halfway_to_higher_note, 0, SCREEN_WIDTH, note);

}

int getXValForLED(float note, std::string &current_closest_note){
	int index = findClosestIndex(allPossibleFreqs, NUM_FREQS, note);
	float closest_note_actual = allPossibleFreqs[index];

	int note_index = index % 12;

	int note_octave = index / 12 + 1;

	int higher_note_index = std::min(index + 1, NUM_FREQS - 1);
	int lower_note_index = std::max(index - 1, 0);

	float halfway_to_higher_note = (allPossibleFreqs[higher_note_index] - closest_note_actual) / 2;
	float halfway_to_lower_note = (closest_note_actual - allPossibleFreqs[lower_note_index]) / 2;

	int x_pos_screen = mapFloatToInt(closest_note_actual - halfway_to_lower_note, closest_note_actual + halfway_to_higher_note, 0, SCREEN_WIDTH, note);

  current_closest_note = NOTES[note_index] + std::to_string(note_octave);
	return x_pos_screen;
}
