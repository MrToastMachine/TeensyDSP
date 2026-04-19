#include <math.h>
#include <stdio.h>
#include <complex>



int findClosestIndex(float arr[], uint8_t arrLen, float target){
    float res = arr[0];
    int lo = 0, hi = arrLen;

    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;

        // Update res if mid is closer to target
        if (abs(arr[mid] - target) < abs(res - target)) {
            res = arr[mid];

            // In case of a tie, prefer larger value
        }
        else if (abs(arr[mid] - target) == abs(res - target)) {
            res = fmax(res, arr[mid]);
        }

        if (arr[mid] == target) {
            return arr[mid];
        }
        else if (arr[mid] < target) {
            lo = mid + 1;
        }
        else {
            hi = mid - 1;
        }
    }

    return res;
}

int main(){

	printf("Ass\n");
	return 0;	
}

