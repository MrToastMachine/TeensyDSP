/*
 * PreProcessing.c
 *
 *  Created on: Jan 23, 2025
 *      Author: AlexOMahony
 */


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "PreProcessing.h"
#include "PreProcData.h"

#include "stm32n657xx.h"
#include "stm32n6xx_hal.h"

#define IIR_a 9875
#define IIR_a_float 0.988

int channelUnderTest = 0;

long channels[NUM_CHANNELS+1];
long montage[NUM_CHANNELS];
long montage_hp[NUM_CHANNELS];
long montage_lp[NUM_CHANNELS];
float montage_hp_last[NUM_CHANNELS] = {0};
float montage_prev_input[NUM_CHANNELS] = {0};

sBipolarMontage montage_databank1 = {0}; // Post Filtering and Downsampling
sBipolarMontage montage_databank2 = {0};
sWindowMontage active_window = {0};
sPreProcessOutput pre_processing_output = {0};
sBipolarMontage *bipolar_montage = &montage_databank1;

s_preProcFilter preproc_lp_filters[NUM_CHANNELS];

/* DEBUG VARIABLES */
int numsAdded = 0;
int numDiscards = 0;
int numDatapointsSaved = 0;

/* OB_BOARD DEBUG */
int currentIndex = 0;
volatile uint32_t startCycles;
volatile uint32_t endCycles;
volatile uint32_t elapsedCycles;
volatile float elapsedTime_us;

float dpProcessTimeSum = 0.0;
float allProcessingTimes[N_ROWS] = {0.0};

const int downsample_rate = 5;
volatile unsigned int iData = 0;
int iMontage = 0;
int iWindow = 0;

int numLines = 0;
int numWindows = 0;
int currentWindow = 0;
int newWindowReady = 0;

//int outputWindows[WINDOW_BUFFER][NUM_CHANNELS * 2 * WINDOW_LENGTH];

// Low Pass Filter Taps (63 Taps)
static const int preproc_filter_taps[PREPROC_FILTER_TAPS] = {
  -1359216,
  -1784769,
  -2103490,
  -2233167,
  -2032550,
  -1333682,
  0,
  2002311,
  4538442,
  7265540,
  9637982,
  10967950,
  10540563,
  7769885,
  2370349,
  -5489816,
  -15088501,
  -25133242,
  -33846629,
  -39154946,
  -38962111,
  -31473120,
  -15518135,
  9176962,
  41823732,
  80545104,
  122506507,
  164195948,
  201820112,
  231763236,
  251043270,
  257698038,
  251043270,
  231763236,
  201820112,
  164195948,
  122506507,
  80545104,
  41823732,
  9176962,
  -15518135,
  -31473120,
  -38962111,
  -39154946,
  -33846629,
  -25133242,
  -15088501,
  -5489816,
  2370349,
  7769885,
  10540563,
  10967950,
  9637982,
  7265540,
  4538442,
  2002311,
  0,
  -1333682,
  -2032550,
  -2233167,
  -2103490,
  -1784769,
  -1359216
};

// Function to calculate the mean
double CalculateMean(volatile long arr[]) {
  double sum = 0.0;
  int iOffset = MAF_WINDOW_LEN/2;

  for (int i = 0; i < WINDOW_LENGTH; i++) {
    sum += (double)arr[i + iOffset];
  }
  return sum / (double)WINDOW_LENGTH;
}

// Function to calculate the standard deviation
double CalculateStdDev(volatile long arr[]) {
  double mean = CalculateMean(arr);
  double sum_squared_diff = 0.0;
  int iOffset = MAF_WINDOW_LEN/2;

  // Calculate the sum of squared differences from the mean
  for (int i = 0; i < WINDOW_LENGTH; i++) {
    sum_squared_diff += ((double)arr[i+iOffset] - mean) * ((double)arr[i+iOffset] - mean);
  }

  // Calculate and return the standard deviation
  return sqrt(sum_squared_diff / (double)WINDOW_LENGTH);
}

double CalculateMAA_old(volatile long arr[]){
  double sum = 0.0;
  int iOffset = MAF_WINDOW_LEN/2;

  for (int i = 0; i < WINDOW_LENGTH; i++) {
    sum += abs((double)arr[i+iOffset]);
  }
  return sum / (double)WINDOW_LENGTH;
}

double CalculateMAA(volatile long arr[]) {
    double sum = 0.0;
    int startIndex = MAF_WINDOW_LEN/2;

    for (int i = 0; i < WINDOW_LENGTH; i++) {
        sum += fabs((double)arr[startIndex + i]);
    }
    return sum / (double)WINDOW_LENGTH;
}

bool isAllZeros(volatile long arr[]){
  int iOffset = MAF_WINDOW_LEN/2;

  for (int i = 0; i < WINDOW_LENGTH; i++) {
    if (arr[i+iOffset] != 0){
      return false;
    }
  }
  return true;
}

void PreProcLPFilter_init(s_preProcFilter *f) {
  int i;
  for (i = 0; i < PREPROC_FILTER_TAPS; ++i)
    f->history[i] = 0;
  f->last_index = 0;
}

void PreProcLPFilter_put(s_preProcFilter *f, int input) {
  f->history[f->last_index++] = input;

  if (f->last_index == PREPROC_FILTER_TAPS)
    f->last_index = 0;
}

int PreProcLPFilter_get(s_preProcFilter* f) {
  long long acc = 0;
  int index = f->last_index, i;
  for(i = 0; i < PREPROC_FILTER_TAPS; ++i) {
    index = index != 0 ? index-1 : PREPROC_FILTER_TAPS-1;
    acc += f->history[index] * preproc_filter_taps[i];
  };

  return acc >> 31;
}

int PreProcLPFilter_get2(s_preProcFilter *f) {
  long long acc = 0;
  int index = f->last_index, i;
  int index_sym = index - 1;

  for (i = 0; i < (PREPROC_FILTER_TAPS + 1) / 2 - 1; ++i) {
    index_sym =
        (index_sym != PREPROC_FILTER_TAPS - 1) ? index_sym + 1 : 0;
    index = (index != 0) ? index - 1 : PREPROC_FILTER_TAPS - 1;
    //		index_sym = (index < IMPEDANCECHECKFILTER_TAP_NUM-1-(2*i)) ?
    // index +1 +2*i : index - IMPEDANCECHECKFILTER_TAP_NUM+1 + 2*i;

    acc += ((long long)f->history[index] + (long long)f->history[index_sym]) *
           preproc_filter_taps[i];
  }
  index = (index != 0) ? index - 1 : PREPROC_FILTER_TAPS - 1;
  acc += (long long)f->history[index] * preproc_filter_taps[i];

  return acc >> 31;
}

int8_t convertTo8BitRange(int32_t value) {
    // Define the maximum input range

    // Clamp the input value to the valid range
    if (value > CLIPPING_LIM_INT) {
        value = CLIPPING_LIM_INT;
    } else if (value < -CLIPPING_LIM_INT) {
        value = -CLIPPING_LIM_INT;
    }

    // Perform the scaling to the range -1 to 1
    int8_t result = (int8_t)((value * 127) / CLIPPING_LIM_INT);

    return result;
}

void CreateMontage(volatile long channels[], volatile long result[]) {
  result[0] = channels[6] - channels[4];  // F4-C4
  result[1] = channels[4] - channels[0];  // C4-O2
  result[2] = channels[7] - channels[5];  // F3-C3
  result[3] = channels[5] - channels[1];  // C3-O1
  result[4] = channels[2] - channels[4];  // T4-C4

  // TODO Change this back to normal for actual firmware code
  // result[5] = channels[4];                // C4-Cz
  // result[6] = -channels[5];               // Cz-C3
  result[5] = channels[4] - channels[8];                // C4-Cz
  result[6] = channels[8] - channels[5];               // Cz-C3

  result[7] = channels[5] - channels[3];  // C3-T3

  if (0){
    printf("F4-C4: %d\n", result[0]);
  }

}

void HP_FilterSignal_float(volatile long montage_input[], volatile long filter_output[]){
  // channels[i] -> Current sample before filtering
  // PP_channel_hp[i] -> output of high pass filter (Pre-Processing)
  // PP_channel_hp_last[i -> Prev output of hp

  /** High Pass Filter at 0.5Hz
   * IIR Filter
   * y[n] = a*y[n-1] + 1000 * (x[n] - y[n-1])
   */

  for (int i = 0; i < NUM_CHANNELS; i++)
  {
    // filter_output[i] = montage_hp_last[i] * IIR_a + 10000 * (montage_input[i] - montage_prev_input[i]);
    // filter_output[i] /= 10000;

    float temp = montage_hp_last[i] * IIR_a_float + ((float)montage_input[i] - montage_prev_input[i]);
    filter_output[i] = (int)temp;

    // montage_hp_last[i] = filter_output[i];
    montage_hp_last[i] = temp;
    montage_prev_input[i] = montage_input[i];
  }

}

void HP_FilterSignal(volatile long montage_input[], volatile long filter_output[]) {
    for (int i = 0; i < NUM_CHANNELS; i++) {
        // Use fixed-point arithmetic instead of float
        long temp = (montage_hp_last[i] * IIR_a) / 1000 +
                    (montage_input[i] - montage_prev_input[i]);

        filter_output[i] = temp;
        montage_hp_last[i] = temp;
        montage_prev_input[i] = montage_input[i];
    }
}
void LP_FilterSignal(volatile long montage_input[], volatile long filter_output[]){

  /** Low Pass Filter at 15Hz
   * FIR filter with 63 Taps
   */
  for (int i = 0; i < NUM_CHANNELS; i++){
    PreProcLPFilter_put(&preproc_lp_filters[i], montage_input[i]);
    filter_output[i] = PreProcLPFilter_get2(&preproc_lp_filters[i]);

    // Uncomment to bypass LP Filter
    // filter_output[i] = montage[i];
  }
}

int DownsampleSignal(volatile long inputChannels[]){

  // printf("iData %d\t", iData);
  /* Downsampling to 50Hz */
  if ((iData + 5) % downsample_rate == 0){
    // printf("Taking this sample [%d]...\n", iData);
    switch (iMontage)
    {
    case NUM_MONTAGE_HIST_POINTS: {
      // Limit of current databank reached, need to switch

      iMontage = 0;

      if (bipolar_montage == &montage_databank1){
        // Switch to databank 2
        bipolar_montage = &montage_databank2;
      }
      else if (bipolar_montage == &montage_databank2){
        // Switch to databank 1
        bipolar_montage = &montage_databank1;
      }
      break;
    }
    default:
      break;
    }

    // Add filtered & downsampled sample to databank
    for (int i = 0; i < NUM_CHANNELS; i++)
    {
      bipolar_montage->channel[i][iMontage] = inputChannels[i];
    }

    iMontage++;
    return 1;
  }
  return 0;
}

void CreateWindow(sWindowMontage *window_data){
  // printf("Creating window after adding %d nums\n", numsAdded);

  int index, sample;
  int montageOffset;
  int temp;

  volatile sBipolarMontage *alt_montage;

  if (bipolar_montage == &montage_databank1){
    alt_montage = &montage_databank2;
  }
  else if (bipolar_montage == &montage_databank2){
    alt_montage = &montage_databank1;
  }

  for (int i = 0; i < NUM_CHANNELS; i++){
    for (index = 0; index < WINDOW_HIST_LEN; index++)
    {
      montageOffset = iMontage - index - 1;
      if (montageOffset < 0){
        // Need to go back to prev databank to get points
        sample = alt_montage->channel[i][NUM_MONTAGE_HIST_POINTS + montageOffset];
        // printf("[iData: %5d] alt sample: %5d\n", iData, sample);
      }
      else {
        // Can get point from current databank
        sample = bipolar_montage->channel[i][montageOffset];
      }
      temp = WINDOW_HIST_LEN - (index + 1);
      window_data->channel[i][temp] = sample;
    }
  }

  numWindows++;
//  if (numWindows % 1000 == 0 || 1){
//    printf("Num Windows Created: %4d\n", numWindows);
//    // printf("Num Windows Created: %4d | iData = %5d | iMontage = %5d | iWindow = %d | numLines = %5d\n", numWindows, iData, iMontage, iWindow, numLines);
//  }
  newWindowReady = 1;
}

bool DiscardWindowNoise(sWindowMontage *window_data){
  double mean_abs_amp, stddev;

  int maa_discard_count = 0;
  int std_discard_count = 0;
  int ch_zero_count = 0;
  int num_channels_for_discard = 4; // From Sergios preprocessing slides

  int jOffset = MAF_WINDOW_LEN/2;

  // for this step only care about central 800 datapoints
  for (int i = 0; i < NUM_CHANNELS; i++)
  {
    mean_abs_amp = CalculateMAA(window_data->channel[i]);
    stddev = CalculateStdDev(window_data->channel[i]);

    /* DEBUGGING */
    // printf("[ch%d]\tMAA:     %f\n", i, mean_abs_amp);
    // printf("[ch%d]\tStd_Dev: %f\n\n", i, stddev);

    if (mean_abs_amp > DISCARD_LIM_INT){
      maa_discard_count++;
    }
    if (stddev > DISCARD_LIM_INT){
      std_discard_count++;
    }
    if (isAllZeros(window_data->channel[i])){
      // ch_zero_count++;
    }

    if (maa_discard_count >= num_channels_for_discard ||
        std_discard_count >= num_channels_for_discard ||
        ch_zero_count > num_channels_for_discard) {
      numDiscards++;
      // printf("[DISCARD] Window %5d\n", numWindows);
      // Set all channels in this window to zero
      memset(window_data->channel, 0, sizeof(window_data->channel));
      // printf("[DISCARDING WINDOW #%5d] 4 or more channels above limit!\n", numWindows);
      return true;

    } else if(mean_abs_amp > MAX_MAA_INT) {
      // Set current channel in window to zeros
      memset(window_data->channel[i], 0, sizeof(window_data->channel[i]));
      // printf("[DISCARDING CHANNEL #%d - WIN  #%5d] MAA above limit!\n", i+1, numWindows);
      // printf("\tMAA = %f\n", mean_abs_amp * SCALE_uV_TO_INT);
    } else if (stddev > MAX_STDDEV_INT) {
      memset(window_data->channel[i], 0, sizeof(window_data->channel[i]));
      // printf("[DISCARDING CHANNEL #%d - WIN  #%4d] STD above limit!\n", i+1, numWindows);
      // printf("\tMAA = %f\n", stddev * SCALE_uV_TO_INT);
    }
  }
  return false;
}

void DetrendWindow(int bypass){
  int half_window = MAF_WINDOW_LEN/2;
  int val = 0;
  float sum = 0;
  float avg = 0;


  for (int i = 0; i < NUM_CHANNELS; i++)
  {
    for (int j = 0; j < WINDOW_LENGTH; j++)
    {

      sum = 0;
      for (int index = 0; index < MAF_WINDOW_LEN+1; index++){
        val = active_window.channel[i][index];
        sum += val;

      }

      avg = (float)sum / (float)(MAF_WINDOW_LEN + 1);


      if (bypass){
        pre_processing_output.channel[i][j] = active_window.channel[i][j+MAF_WINDOW_LEN/2];
      }
      else {
        pre_processing_output.channel[i][j] = active_window.channel[i][j+MAF_WINDOW_LEN/2] - (int)avg;
      }
    }
  }

}

void DetrendWindow2()
{

  int trendData[800] = {0};

  int half_window = MAF_WINDOW_LEN / 2;

  for (int i = 0; i < NUM_CHANNELS; i++)
  {
    for (int j = 0; j < WINDOW_LENGTH; j++)
    {
      int sum = 0;

      // Calculate moving average
      for (int k = 0; k < MAF_WINDOW_LEN+1; k++)
      {
        int index = j + k;
        if (index < 0 || index >= WINDOW_LENGTH)
        {
          // Handle out-of-bounds index (if necessary)
          continue;
        }
        sum += active_window.channel[i][index];
      }
      float avg = (float)sum / MAF_WINDOW_LEN+1;

      int center_index = j + half_window;
      if (center_index >= 0 && center_index < WINDOW_HIST_LEN)
      {
        pre_processing_output.channel[i][j] = active_window.channel[i][center_index] - (int)avg;

      }
    }
  }
}

void DetrendWindow3(sWindowMontage *window_data)
{
  int half_window = MAF_WINDOW_LEN / 2;


  for (int i = 0; i < NUM_CHANNELS; i++)
  {
    for (int j = 0; j < WINDOW_LENGTH; j++)
    {
      int sum = 0;

      // j = j + offset;

      // Calculate moving average
      for (int k = 0; k < MAF_WINDOW_LEN+1; k++)
      {
        int index = j + k;
        if (index < 0 || index >= WINDOW_HIST_LEN)
        {
          // Handle out-of-bounds index (if necessary)
          continue;
        }
        sum += window_data->channel[i][index];
      }
      float avg = (float)sum / MAF_WINDOW_LEN+1;

      int center_index = j + half_window;
      if (center_index >= 0 && center_index < WINDOW_HIST_LEN)
      {
        pre_processing_output.channel[i][j] =
            window_data->channel[i][center_index] - (int)avg;
      }
    }
  }
}

void skipDetrend(){
  int offset = MAF_WINDOW_LEN/2;

  for (int i = 0; i < NUM_CHANNELS; i++)
  {
    for (int j = 0; j < WINDOW_LENGTH; j++)
    {
      pre_processing_output.channel[i][j] = active_window.channel[i][j+offset];
    }
  }
}

void ClipAndNormalise(){
  for (int i = 0; i < NUM_CHANNELS; i++)
  {
    for (int j = 0; j < WINDOW_LENGTH; j++)
    {
      pre_processing_output.channel[i][j] = convertTo8BitRange(pre_processing_output.channel[i][j]);
      // if (pre_processing_output.channel[i][j] > CLIPPING_LIM_INT){
      //   pre_processing_output.channel[i][j] = CLIPPING_LIM_INT;
      // } else if (pre_processing_output.channel[i][j] < -CLIPPING_LIM_INT){
      //   pre_processing_output.channel[i][j] = -CLIPPING_LIM_INT;
      // }
    }
  }
}

void AddFlippedChannels(){
  for (int i = 0; i < NUM_CHANNELS; i++)
  {
    for (int j = 0; j < WINDOW_LENGTH; j++)
    {
      pre_processing_output.channel[i+NUM_CHANNELS][j] = -pre_processing_output.channel[i][j];
    }
  }

}

void FullPreProcessing(){
  // 1. Create Bipolar Montage
  CreateMontage(channels, montage);

  // Now montage contains 8 bipolar channels

  // 2. Filter and Downsample

  LP_FilterSignal(montage, montage_lp);
  HP_FilterSignal_float(montage_lp, montage_hp);

  // int keepDatapoint;
  // keepDatapoint = DownsampleSignal(montage_hp);


  if (DownsampleSignal(montage_hp) != 1) {
    return;
  }

  // 3. Windowing

  // This will run every 1 second (50 samples at 50Hz)
  if (iWindow != WINDOW_OVERLAP-1){
    iWindow++;
    return;
  } else if (iMontage < WINDOW_HIST_LEN-(74/2) && numWindows == 0){
    // ensure first windows has enough datapoints
    // printf("[iData: %4d] not enough [iWindow: %4d]\n", iData, iWindow);
    return;
  }

  // Collected enough samples to fill next window
  // printf("Creating Window at: %d\n", numLines);
  CreateWindow(&active_window);
  newWindowReady = 0;
  iWindow = 0;

//  return;

  // 4. Discard Noise

  if (DiscardWindowNoise(&active_window)){
  	// This is where the window discarding happens
  	// Will have to send Null (-1) value directly to post from here

    numWindows--;
    return;
  }

  // 5. Detrend Windows
  // DetrendWindow(1);
  // DetrendWindow2();
  DetrendWindow3(&active_window);
  // skipDetrend();


  // 6 + 7. Clipping and Normalisation + Quantization

  ClipAndNormalise();

  // 8. Add Vertically flipped data
  AddFlippedChannels();

//  if (numWindows == 15){
//  	printf("[");
//  	for (int i = 0; i < WINDOW_LENGTH - 1; i++){
//  		printf("%d, ", pre_processing_output.channel[0][i]);
//  	}
//  	printf("%d]\n", pre_processing_output.channel[0][WINDOW_LENGTH-1]);
//  	printf("Window Print Complete...\n");
//  }

//  WindowReady();

}

void WindowReady(){
  for (int i = 0; i < 16; i++)
  {
  	printf("%6d [", i);
    for (int j = 0; j < WINDOW_LENGTH; j++)
    {
//      outputWindows[currentWindow][index] = pre_processing_output.channel[i][j];
      printf("%d, ", pre_processing_output.channel[i][j]);

      if (j == WINDOW_LENGTH-1){
      	printf("]\n");
      }
    }
  }
}

void ReadFromDataFile(){

  int row = 0;  // Row counter

	for(int i = 0; i < N_ROWS; i++){
		currentIndex = i;
//		printf("%6d [", i);
		for(int j = 0; j < N_COLS-1; j++){
//			printf("%d, ", EDF_Data[i][j]);
			channels[j] = EDF_Data[i][j];
		}
//		printf("%d]\n", EDF_Data[i][N_COLS-1]);



		// Measure execution time
		startCycles = TIM2->CNT;

//			x();  // Call the function to be measured
		FullPreProcessing();

//		HAL_Delay(2);

		endCycles = TIM2->CNT;

    if (endCycles > startCycles){
		// Calculate elapsed cycles
      elapsedCycles = endCycles - startCycles;

      elapsedTime_us = (float)elapsedCycles / 64.0f;
      
      allProcessingTimes[i] = elapsedTime_us; 
      dpProcessTimeSum += elapsedTime_us;
    } else {  
      allProcessingTimes[i] = allProcessingTimes[i-1]; 
      dpProcessTimeSum += allProcessingTimes[i-1];
    }

		printf("[%4d]> Time taken: %f uS\n",i, elapsedTime_us);

		iData++;
	}

  // Calculate average time for datapoint to be processed
  float avgTime = dpProcessTimeSum / (float)N_ROWS;

  printf("Average Time per Datapoint: %f uS\n", avgTime);

  float sum = 0.0f;
  for (int i = 0; i < N_ROWS; i++)
  {
    if (allProcessingTimes[i] > 1000){
      printf("[%4d] Above Lim: %fuS\n", i, allProcessingTimes[i]);
    }
    sum += allProcessingTimes[i];
  }

  float avg = sum / 8000;

  printf("Other Avg: %fuS\n", avg);
  

}

void ReadFromFlash(){

  int row = 0;  // Row counter

	for(int i = 0; i < N_ROWS; i++){
		currentIndex = i;
		for(int j = 0; j < N_COLS-1; j++){
			channels[j] = EDF_Data[i][j];
		}

		FullPreProcessing();

		iData++;
	}

  // Calculate average time for datapoint to be processed
  float avgTime = dpProcessTimeSum / (float)N_ROWS;

  printf("Average Time per Datapoint: %f uS\n", avgTime);

  float sum = 0.0f;
  for (int i = 0; i < N_ROWS; i++)
  {
    if (allProcessingTimes[i] > 1000){
      printf("[%4d] Above Lim: %fuS\n", i, allProcessingTimes[i]);
    }
    sum += allProcessingTimes[i];
  }

  float avg = sum / 8000;

  printf("Other Avg: %fuS\n", avg);


}

void timingTest(){

	   printf("Starting timing test...\n");

	   while(1){
			//	compute_test();
	   startCycles = TIM2->CNT;
			HAL_Delay(1000);
			printf("Here\n");
	   endCycles = TIM2->CNT;

	   elapsedCycles = endCycles - startCycles;

	   printf("Num ticks: %d\n", elapsedCycles);
	   }

	   return;
}


#ifndef UNIT_TEST
void PreProcessData() {

	// Initialise the PreProcessing LP Filters
	for (int i = 0; i < NUM_CHANNELS; i++) {
		PreProcLPFilter_init(&preproc_lp_filters[i]);
	}

  printf("Starting Data Preprocessing...\n");

//  ReadFromDataFile();

  ReadFromFlash();

  printf("Finished Data Preprocessing...\n");

}

#endif
