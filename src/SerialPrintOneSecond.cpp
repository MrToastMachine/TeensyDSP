// #include <Arduino.h>
// // Teensy 4.1 - Sample A0 at ~48 kHz for 1 second and print as CSV

// const uint32_t SAMPLE_RATE   = 48000;            // Hz
// const uint32_t DURATION_SEC  = 1;                // seconds
// const uint32_t NUM_SAMPLES   = SAMPLE_RATE * DURATION_SEC;
// const uint32_t SAMPLE_PERIOD_US = 1000000UL / SAMPLE_RATE; // ~20 us

// uint16_t samples[NUM_SAMPLES];

// void setup() {
//   // Start Serial
//   Serial.begin(115200);
//   while (!Serial) {
//     // wait for USB Serial to be ready
//   }

//   // Configure ADC
//   analogReadResolution(12);      // 12-bit (0–4095)
//   analogReadAveraging(1);        // no averaging for speed

//   // Optional: throw away first read
//   analogRead(A0);

//   // --- ACQUIRE SAMPLES ---
//   Serial.println("Starting capture...");

//   uint32_t t = micros();
//   for (uint32_t i = 0; i < NUM_SAMPLES; i++) {
//     // Wait until next sample time
//     while ((micros() - t) < SAMPLE_PERIOD_US) {
//       // busy wait
//     }
//     t += SAMPLE_PERIOD_US;

//     samples[i] = analogRead(A0);
//   }

//   Serial.println("Capture complete. Sending data as CSV:");
//   Serial.println("index,sample");  // CSV header

//   // --- SEND DATA ---
//   for (uint32_t i = 0; i < NUM_SAMPLES; i++) {
//     Serial.print(i);
//     Serial.print(',');
//     Serial.println(samples[i]);
//   }

//   Serial.println("Done.");
// }

// void loop() {
//   // Do nothing, one-shot capture
// }
