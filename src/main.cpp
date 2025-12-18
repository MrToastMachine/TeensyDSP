// // Simple WAV recorder for Teensy 4.1
// // - Mono input (A0 by default, change if needed)
// // - 16-bit, 44.1 kHz
// // - Writes to built-in microSD as "RECORD.WAV"

// #include <Arduino.h>
// #include <Audio.h>
// #include <Wire.h>
// #include <SPI.h>
// #include <SD.h>


// // OLED display libraries
// #include <SPI.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
// #include <math.h>
// #include <stdio.h>

// #define SCREEN_WIDTH 128 // OLED display width, in pixels
// #define SCREEN_HEIGHT 64 // OLED display height, in pixels

// // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// #define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// // END OF OLED STUFF


// // -------- Audio Objects --------
// // Use analog input (mono). On Teensy 4.1, A0 is a good default.
// // You can change AUDIO_INPUT_PIN below if you like.
// #define AUDIO_INPUT_PIN A0

// AudioInputAnalog         adc1(AUDIO_INPUT_PIN);  // mono analog in
// AudioRecordQueue         queue1;                 // queue for recording
// AudioConnection          patchCord1(adc1, queue1);

// // No audio shield, so no AudioControl object needed

// // -------- SD / WAV Globals --------
// File audioFile;

// const uint32_t SAMPLE_RATE      = 44100;
// const uint16_t BITS_PER_SAMPLE  = 16;
// const uint16_t NUM_CHANNELS     = 1;    // mono

// // We record for a fixed duration (milliseconds)
// const uint32_t RECORD_TIME_MS   = 10000;  // 10 seconds (change as needed)

// elapsedMillis recordTimer;
// bool isRecording = false;

// // We'll count how many samples we wrote to update the WAV header at the end.
// uint32_t totalSamplesWritten = 0;

// // -------- Helpers to write little-endian values --------
// void writeLittleEndian16(File &f, uint16_t value) {
//   uint8_t b[2];
//   b[0] = value & 0xFF;
//   b[1] = (value >> 8) & 0xFF;
//   f.write(b, 2);
// }

// void writeLittleEndian32(File &f, uint32_t value) {
//   uint8_t b[4];
//   b[0] = value & 0xFF;
//   b[1] = (value >> 8) & 0xFF;
//   b[2] = (value >> 16) & 0xFF;
//   b[3] = (value >> 24) & 0xFF;
//   f.write(b, 4);
// }

// // -------- Write initial (placeholder) WAV header --------
// void writeWavHeader(File &f) {
//   // We'll write a standard 44-byte header.
//   // Sizes will be patched when we're done recording.

//   // RIFF chunk descriptor
//   f.write("RIFF", 4);
//   writeLittleEndian32(f, 0);  // Placeholder for chunk size (file size - 8)

//   // WAVE header
//   f.write("WAVE", 4);

//   // fmt subchunk
//   f.write("fmt ", 4);
//   writeLittleEndian32(f, 16);                 // Subchunk1Size = 16 for PCM
//   writeLittleEndian16(f, 1);                  // AudioFormat = 1 (PCM)
//   writeLittleEndian16(f, NUM_CHANNELS);       // NumChannels
//   writeLittleEndian32(f, SAMPLE_RATE);        // SampleRate

//   uint32_t byteRate = SAMPLE_RATE * NUM_CHANNELS * (BITS_PER_SAMPLE / 8);
//   writeLittleEndian32(f, byteRate);           // ByteRate

//   uint16_t blockAlign = NUM_CHANNELS * (BITS_PER_SAMPLE / 8);
//   writeLittleEndian16(f, blockAlign);         // BlockAlign

//   writeLittleEndian16(f, BITS_PER_SAMPLE);    // BitsPerSample

//   // data subchunk
//   f.write("data", 4);
//   writeLittleEndian32(f, 0);  // Placeholder for Subchunk2Size (data size)
// }

// // -------- Patch WAV header when recording is done --------
// void finalizeWavHeader(File &f, uint32_t totalSamples) {
//   uint32_t dataSize = totalSamples * NUM_CHANNELS * (BITS_PER_SAMPLE / 8);
//   uint32_t riffSize = 36 + dataSize;  // 4 + (8 + Subchunk1Size) + (8 + Subchunk2Size)

//   // Patch RIFF chunk size at offset 4
//   f.seek(4);
//   writeLittleEndian32(f, riffSize);

//   // Patch data subchunk size at offset 40
//   f.seek(40);
//   writeLittleEndian32(f, dataSize);
// }

// // -------- Setup --------
// void setup() {
//   Serial.begin(115200);


//   // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
//   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
//     Serial.println(F("SSD1306 allocation failed"));
//     for(;;); // Don't proceed, loop forever
//   }

//   // Show initial display buffer contents on the screen --
//   // the library initializes this with an Adafruit splash screen.
//   display.display();
//   delay(2000); // Pause for 2 seconds

//   // Clear the buffer
//   display.clearDisplay();

//   // Draw a single pixel in white
//   display.drawPixel(10, 10, WHITE);

//   // Show the display buffer on the screen. You MUST call display() after
//   // drawing commands to make them visible on screen!
//   display.display();
//   // delay(2000);


//   while (!Serial) {
//     // wait for USB Serial to open
//   }

//   Serial.println("Teensy 4.1 WAV Recorder starting...");

//   // Allocate audio memory blocks (tune as needed)
//   AudioMemory(12);

//   // Initialize SD card (built-in on Teensy 4.1)
//   if (!SD.begin(BUILTIN_SDCARD)) {
//     Serial.println("SD.begin(BUILTIN_SDCARD) failed!");
//     while (1) {
//       delay(500);
//     }
//   }
//   Serial.println("SD card initialized.");

//   // Open WAV file for writing
//   audioFile = SD.open("RECORD.WAV", FILE_WRITE);
//   if (!audioFile) {
//     Serial.println("Failed to open RECORD.WAV for writing.");
//     while (1) {
//       delay(500);
//     }
//   }

//   // Write placeholder header
//   writeWavHeader(audioFile);
//   audioFile.flush();
//   Serial.println("WAV header written.");

//   // Start recording
//   totalSamplesWritten = 0;
//   queue1.begin();
//   isRecording = true;
//   recordTimer = 0;

//   Serial.print("Recording for ");
//   Serial.print(RECORD_TIME_MS / 1000.0f);
//   Serial.println(" seconds...");
// }

// // -------- Main loop --------
// void loop() {
  

//   if (isRecording) {
//     // Grab audio blocks from the queue and write them to SD
//     while (queue1.available() > 0) {
//       // Each block from AudioRecordQueue is 128 samples of 16-bit data
//       int16_t *buffer = (int16_t *)queue1.readBuffer();
//       if (buffer) {
//         // Write raw PCM samples to file
//         size_t bytesToWrite = 128 * sizeof(int16_t) * NUM_CHANNELS;  // mono
//         audioFile.write((byte *)buffer, bytesToWrite);

//         totalSamplesWritten += 128;  // 128 samples in this block
//       }
//       queue1.freeBuffer();
//     }

//     // Stop after RECORD_TIME_MS
//     if (recordTimer >= RECORD_TIME_MS) {
//       Serial.println("Recording complete, stopping...");
//       queue1.end();
//       isRecording = false;

//       // Finalize header with correct sizes
//       finalizeWavHeader(audioFile, totalSamplesWritten);

//       audioFile.flush();
//       audioFile.close();
//       Serial.println("WAV file closed. Done.");

//       // Stop doing anything else
//       while (1) {
//         delay(1000);
//       }
//     }
//   }
// }
