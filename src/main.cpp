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
File audioFile;

const uint32_t SAMPLE_RATE      = 44100;
const uint16_t BITS_PER_SAMPLE  = 16;
const uint16_t NUM_CHANNELS     = 1;    // mono

bool isRecording = false;


// FFT stuff

const int NUM_SAMPLES = 8192;

using Complex = std::complex<double>;
Complex sample_bank[NUM_SAMPLES];
Complex fft_bins[NUM_SAMPLES];
double abs_fft_bins[NUM_SAMPLES];

int getMaxIndex(const double* input_arr, int n){
	if (n < 1){
		return 0;
	}

	int current_max_index = -1;
	double current_max_val = -100.0;

	for (int i = 0; i < NUM_SAMPLES; i++) {
		if (input_arr[i] > current_max_val){
			current_max_val = input_arr[i];
			current_max_index = i;
		}
	}

	return current_max_index;
}

void fft_arr(const Complex* samples, Complex* result, int n) {
    // Base case
    if (n == 1) {
        result[0] = samples[0];
        return;
    }

    // Split into even and odd
    Complex* even_samples = new Complex[n / 2];
    Complex* odd_samples  = new Complex[n / 2];

    for (int i = 0; i < n / 2; ++i) {
        even_samples[i] = samples[2 * i];
        odd_samples[i]  = samples[2 * i + 1];
    }

    // Recurse into output buffers
    Complex* even_fft = new Complex[n / 2];
    Complex* odd_fft  = new Complex[n / 2];

    fft_arr(even_samples, even_fft, n / 2);
    fft_arr(odd_samples,  odd_fft,  n / 2);

    // Combine
    for (int k = 0; k < n / 2; ++k) {
        Complex t = std::polar(1.0, -2.0 * M_PI * k / n) * odd_fft[k];
        result[k]         = even_fft[k] + t;
        result[k + n / 2] = even_fft[k] - t;
    }

    delete[] even_samples;
    delete[] odd_samples;
    delete[] even_fft;
    delete[] odd_fft;
}

void arr_computeFFT(const std::string& filename){

	std::vector<Complex> wavData;

	loadWavFile(filename, wavData, NUM_SAMPLES);

	fft_arr(sample_bank, fft_bins, NUM_SAMPLES);

	// Print results (for testing)
	for (size_t i = 0; i < NUM_SAMPLES; ++i) {
		// std::cout << "F[" << i << "] = " << fft_bins[i] << std::endl;
		double new_val = std::abs(fft_bins[i]);
		
		abs_fft_bins[i] = new_val;

		std::cout << i << ", " << new_val << std::endl;
	}

	int max_index = getMaxIndex(abs_fft_bins, NUM_SAMPLES);

	double strongest_freq = (double)max_index * ((double)fs / (double)NUM_SAMPLES);

	
	std::cout << "Max index: " << max_index << std::endl;
	std::cout << "Strongest Freq: " << strongest_freq << std::endl;
		
}

void addBufferValsToDatabank(int16_t *buff, int buff_size){
  for (int i = 0; i < buff_size; i++)
  {
    fft_databank[i + iData - 1] = buff[i];
  }
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


        // TODO: CHANGE THIS LINE
        audioFile.write((byte *)buffer, bytesToWrite);

        // Check if enough vals in analysis block
        // Append all vals in buffer to my analysis block

        addBufferValsToDatabank(buffer, 128);
        
        iData += 128;

        if (iData == FFT_DATABANK_SIZE){
          // ready to complete fft

          computeFFT();

          iData = 0;
        }



      }
      queue1.freeBuffer();
    }
  }
}
