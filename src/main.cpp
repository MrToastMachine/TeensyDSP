// Simple WAV recorder for Teensy 4.1
// - Mono input (A0 by default, change if needed)
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

const uint16_t FFT_DATABANK_SIZE = 8192;
std::vector<std::complex<double>> fft_databank(FFT_DATABANK_SIZE);
std::vector<std::complex<double>> fft_output_bins(FFT_DATABANK_SIZE);

bool isRecording = false;

uint16_t iData;

std::vector<std::complex<double>> fft(const std::vector<std::complex<double>>& samples) {
    // Get n, the number of samples 
    int n = static_cast<int>(samples.size());

    // Base case for recursion 
    if (n == 1) {
        return samples;
    }

    // Split samples into even and odd arrays
    std::vector<std::complex<double>> even_samples(n / 2);
    std::vector<std::complex<double>> odd_samples(n / 2);

    for (int i = 0; i < n / 2; ++i) {
        even_samples[i] = samples[2 * i];
        odd_samples[i] = samples[2 * i + 1];
    }

    // Recursively run the above lines
    std::vector<std::complex<double>> even_fft = fft(even_samples);
    std::vector<std::complex<double>> odd_fft = fft(odd_samples);

    // Combine the values at each level
    std::vector<std::complex<double>> result(n);
    for (int k = 0; k < n / 2; ++k) {
        std::complex<double> t =
            std::polar(1.0, -2 * M_PI * k / n) * odd_fft[k];
        result[k] = even_fft[k] + t;
        result[k + n / 2] = even_fft[k] - t; // Takes advantage of symmetry
    }

    return result;
}

void computeFFT(){

  fft_output_bins = fft(fft_databank);

	// Print results (for testing)

  Serial.print('[');
	for (size_t i = 0; i < fft_output_bins.size() - 1; ++i) {
			// std::cout << "F[" << i << "] = " << output_freq_bins[i] << std::endl;
      Serial.print(std::abs(fft_output_bins[i]));
      Serial.print(", ");
	}
  Serial.print(std::abs(fft_output_bins[fft_output_bins.size()-1]));
  Serial.println("]");

  // Print output databins

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
