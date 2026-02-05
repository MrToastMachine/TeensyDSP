// #include <Arduino.h>

// static constexpr uint8_t DAC_PIN_0 = 0; // LSB
// static constexpr uint8_t DAC_PIN_1 = 1;
// static constexpr uint8_t DAC_PIN_2 = 2;
// static constexpr uint8_t DAC_PIN_3 = 3;
// static constexpr uint8_t DAC_PIN_4 = 4;
// static constexpr uint8_t DAC_PIN_5 = 5;
// static constexpr uint8_t DAC_PIN_6 = 6;
// static constexpr uint8_t DAC_PIN_7 = 7; // MSB

// static constexpr uint32_t TABLE_SIZE = 256;
// static uint8_t sineTable[TABLE_SIZE];

// static constexpr float SAMPLE_RATE = 44100.0f;
// static constexpr float PERIOD_US = 1000000.0f / SAMPLE_RATE;

// static volatile float k = 0.0f;
// static float k_step = 0.0f;

// static inline void writeDAC(uint8_t v) {
//   digitalWriteFast(DAC_PIN_0, (v >> 0) & 1);
//   digitalWriteFast(DAC_PIN_1, (v >> 1) & 1);
//   digitalWriteFast(DAC_PIN_2, (v >> 2) & 1);
//   digitalWriteFast(DAC_PIN_3, (v >> 3) & 1);
//   digitalWriteFast(DAC_PIN_4, (v >> 4) & 1);
//   digitalWriteFast(DAC_PIN_5, (v >> 5) & 1);
//   digitalWriteFast(DAC_PIN_6, (v >> 6) & 1);
//   digitalWriteFast(DAC_PIN_7, (v >> 7) & 1);
// }

// IntervalTimer sampleTimer;

// float getKstepFromFreq(float frequency) {
//   return (frequency * (float)TABLE_SIZE) / SAMPLE_RATE;
// }

// void onSampleTick() {
//   int idx = (int)k;                // truncation is fine here
//   idx &= (TABLE_SIZE - 1);         // TABLE_SIZE=256, so this is fast modulo
//   writeDAC(sineTable[idx]);
//   k += k_step;
//   if (k >= TABLE_SIZE) k -= TABLE_SIZE; // keep bounded
// }

// void genTable() {
//   for (uint32_t i = 0; i < TABLE_SIZE; i++) {
//     float x = (2.0f * 3.14159265358979323846f * (float)i) / (float)TABLE_SIZE;
//     float u = (sinf(x) * 0.5f) + 0.5f;
//     int y = (int)lroundf(u * 255.0f);
//     if (y < 0) y = 0;
//     if (y > 255) y = 255;
//     sineTable[i] = (uint8_t)y;
//   }
// }

// void setup() {
//   pinMode(DAC_PIN_0, OUTPUT);
//   pinMode(DAC_PIN_1, OUTPUT);
//   pinMode(DAC_PIN_2, OUTPUT);
//   pinMode(DAC_PIN_3, OUTPUT);
//   pinMode(DAC_PIN_4, OUTPUT);
//   pinMode(DAC_PIN_5, OUTPUT);
//   pinMode(DAC_PIN_6, OUTPUT);
//   pinMode(DAC_PIN_7, OUTPUT);

//   genTable();
//   k_step = (440.0f * (float)TABLE_SIZE) / SAMPLE_RATE;

//   sampleTimer.begin(onSampleTick, PERIOD_US);
// }

// void loop() {
//   // leave empty or do non-time-critical work
// }
