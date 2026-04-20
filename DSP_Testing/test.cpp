#include <math.h>
#include <stdio.h>
#include <complex>

#define NUM_FREQS 49

const float allPossibleFreqs[NUM_FREQS] = { 27.5 ,  29.14,  30.87,  32.7 ,  34.65,  36.71,  38.89,  41.2 ,
        43.65,  46.25,  49.  ,  51.91,  55.  ,  58.27,  61.74,  65.41,
        69.3 ,  73.42,  77.78,  82.41,  87.31,  92.5 ,  98.  , 103.83,
       110.0  , 116.54, 123.47, 130.81, 138.59, 146.83, 155.56, 164.81,
       174.61, 185.  , 196.  , 207.65, 220.  , 233.08, 246.94, 261.63,
       277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392.  , 415.3 ,
       440.0  };

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

int main(int argc, char* argv[]){
	printf("AssBegin\n");

	if (argc < 2){
		printf("not enough arguments\n");
		return 1;
	}

	float test = std::atof(argv[1]);
	
	printf("Arg given -> %f\n", test);


	int index = findClosestIndex(allPossibleFreqs, NUM_FREQS, test);
	printf("Closest index in freq bank -> %d\n", index);

	printf("Closest Frequency -> %f\n", allPossibleFreqs[index]);

	return 0;	
}

