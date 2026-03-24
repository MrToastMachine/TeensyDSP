// Simple WAV recorder for Teensy 4.1 - Mono input (A0 by default, change if needed)
// - 16-bit, 44.1 kHz
// - Writes to built-in microSD as "RECORD.WAV"

#include <Arduino.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>


// OLED display libraries
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
#include <stdio.h>
#include <complex>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// END OF OLED STUFF


// -------- Audio Objects --------
// Use analog input (mono). On Teensy 4.1, A0 is a good default.
// You can change AUDIO_INPUT_PIN below if you like.
#define AUDIO_INPUT_PIN A0

AudioInputAnalog         adc1(AUDIO_INPUT_PIN);  // mono analog in
AudioRecordQueue         queue1;                 // queue for recording
AudioConnection          patchCord1(adc1, queue1);

// No audio shield, so no AudioControl object needed

// -------- SD / WAV Globals --------
const uint32_t SAMPLE_RATE      = 44100;
const uint16_t BITS_PER_SAMPLE  = 16;
const uint16_t NUM_CHANNELS     = 1;    // mono

bool isRecording = false;


// FFT stuff

uint16_t iData = 0;

const int NUM_SAMPLES = 16384;

using Complex = std::complex<float>;
Complex sample_bank[NUM_SAMPLES];
Complex fft_bins[NUM_SAMPLES];
float abs_fft_bins[NUM_SAMPLES];

int getMaxIndex(const float* input_arr, int n){
	if (n < 1){
		return 0;
	}

	int current_max_index = -1;
	float current_max_val = -100.0;

	for (int i = 0; i < NUM_SAMPLES; i++) {
		if (input_arr[i] > current_max_val){
			current_max_val = input_arr[i];
			current_max_index = i;
		}
	}

	return current_max_index;
}

/* brief: Compute the FFT of a signal
  - samples: arr of data points to analyze
  - result: arr to store FFT output bins (same size as samples)
  - n: number of samples (must be a power of 2)÷
  (disclaimer claude did this one - used to be recursive, but I changed it to iterative in-place to save memory and avoid recursion on an embedded system)
*/
void fft_arr(const Complex* samples, Complex* result, int n) {
    // Copy input into result buffer (we work in-place from here)
    for (int i = 0; i < n; i++) {
        result[i] = samples[i];
    }

    // Bit-reversal permutation
    // Reorders the array so that the iterative butterfly stages work correctly.
    // e.g. for n=8: index 1 (001) <-> index 4 (100), index 3 (011) <-> index 6 (110)
    int bits = 0;
    while ((1 << bits) < n) bits++;  // bits = log2(n)

    for (int i = 0; i < n; i++) {
        int rev = 0;
        for (int b = 0; b < bits; b++) {
            if (i & (1 << b))
                rev |= (1 << (bits - 1 - b));
        }
        if (i < rev) std::swap(result[i], result[rev]);
    }

    // Iterative butterfly stages
    // The recursive version naturally split into log2(n) levels.
    // Here we do those same levels explicitly, bottom-up.
    // 'len' is the size of the sub-FFT at each stage: 2, 4, 8, 16 ... up to n
    for (int len = 2; len <= n; len *= 2) {
        // Twiddle factor step: the angle increment for this stage
        float ang = -2.0 * M_PI / len;
        Complex wlen(cos(ang), sin(ang));  // Base twiddle factor for this stage

        // Step through the array in chunks of 'len'
        for (int i = 0; i < n; i += len) {
            Complex w(1.0, 0.0);  // Current twiddle factor, starts at e^0 = 1

            // Butterfly operation on each pair within this chunk
            for (int k = 0; k < len / 2; k++) {
                Complex u = result[i + k];
                Complex t = w * result[i + k + len / 2];

                // Same butterfly math as the recursive version
                result[i + k]           = u + t;
                result[i + k + len / 2] = u - t;

                w *= wlen;  // Advance twiddle factor to next position
            }
        }
    }
}

void arr_computeFFT(){

	fft_arr(sample_bank, fft_bins, NUM_SAMPLES);

  // Print results (for testing)
	for (size_t i = 0; i < NUM_SAMPLES/2; ++i) {
		// std::cout << "F[" << i << "] = " << fft_bins[i] << std::endl;
		float new_val = std::abs(fft_bins[i]);
		
		abs_fft_bins[i] = new_val;
	}

	int max_index = getMaxIndex(abs_fft_bins + 1, NUM_SAMPLES / 2 - 1) + 1;


  float strongest_freq = (float)max_index * ((float)SAMPLE_RATE / (float)NUM_SAMPLES);

	
	Serial.print(">Strongest Freq:");
  Serial.println(strongest_freq);

  /*
	// Print results (for testing)
	for (size_t i = 0; i < NUM_SAMPLES/2; ++i) {
		// std::cout << "F[" << i << "] = " << fft_bins[i] << std::endl;
		float new_val = std::abs(fft_bins[i]);
		
		abs_fft_bins[i] = new_val;

    Serial.print(">i:");
    Serial.println(i);
    Serial.print(">new_val:");
    Serial.println(new_val);
	}
  */

  iData = 0; // reset for next round of data collection

}


void addBufferValsToDatabank(int16_t *buff, int buff_size){
  for (int i = 0; i < buff_size; i++)
  {
    Complex c = Complex(static_cast<float>(buff[i]));
    sample_bank[iData + i] = c;
  }

  iData += buff_size;

}

// -------- Setup --------
void setup() {
  Serial.begin(115200);


  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, WHITE);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  // delay(2000);


  while (!Serial) {
    // wait for USB Serial to open
  }

  Serial.println("Teensy 4.1 WAV Recorder starting...");

  iData = 0;

  // Allocate audio memory blocks (tune as needed)
  AudioMemory(12);

  // Start recording
  queue1.begin();
  isRecording = true;
}



// -------- Main loop --------
void loop() {

  // TODO: Check for button press -> start recording, instead of starting immediately in setup().
  
  if (isRecording) {
    // Grab audio blocks from the queue and write them to SD
    while (queue1.available() > 0) {
      // Each block from AudioRecordQueue is 128 samples of 16-bit data
      int16_t *buffer = (int16_t *)queue1.readBuffer();

      if (buffer) {
        // Write raw PCM samples to file
        // THIS IS ALWAYS 128 SAMPLES, BECAUSE THAT'S THE BLOCK SIZE OF AudioRecordQueue
        size_t bytesToWrite = 128 * sizeof(int16_t) * NUM_CHANNELS;  // mono

        // Check if enough vals in analysis block
        // Append all vals in buffer to my analysis block

        addBufferValsToDatabank(buffer, 128);
        

        if (iData >= NUM_SAMPLES){
          // ready to complete fft

          arr_computeFFT();

        }



      }
      queue1.freeBuffer();
    }
  }
}
