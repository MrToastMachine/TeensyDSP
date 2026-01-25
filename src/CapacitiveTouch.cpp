// #include <Arduino.h>
// #include <Wire.h>

// static const uint8_t ADDR = 0x5B;

// static uint8_t read8(uint8_t reg) {
//   Wire.beginTransmission(ADDR);
//   Wire.write(reg);
//   uint8_t e = Wire.endTransmission(false);
//   if (e != 0) return 0xFF;
//   Wire.requestFrom(ADDR, (uint8_t)1);
//   return Wire.available() ? Wire.read() : 0xFE;
// }

// static uint16_t read16(uint8_t reg) {
//   Wire.beginTransmission(ADDR);
//   Wire.write(reg);
//   uint8_t e = Wire.endTransmission(false);
//   if (e != 0) return 0xFFFF;
//   Wire.requestFrom(ADDR, (uint8_t)2);
//   uint8_t lo = Wire.available() ? Wire.read() : 0;
//   uint8_t hi = Wire.available() ? Wire.read() : 0;
//   return (uint16_t)lo | ((uint16_t)hi << 8);
// }

// static uint8_t write8(uint8_t reg, uint8_t val) {
//   Wire.beginTransmission(ADDR);
//   Wire.write(reg);
//   Wire.write(val);
//   return Wire.endTransmission(true); // 0 = success
// }

// static void dumpAck(const char* name, uint8_t err) {
//   Serial.print(name);
//   Serial.print(" write err=");
//   Serial.println(err);
// }

// void setup() {
//   Serial.begin(115200);
//   delay(200);

//   Wire.begin();
//   Wire.setClock(100000);

//   Serial.println("Soft reset...");
//   dumpAck("SRST(0x80=0x63)", write8(0x80, 0x63));
//   delay(50);

//   Serial.println("Stop mode (ECR=0)...");
//   dumpAck("ECR stop", write8(0x5E, 0x00));
//   delay(10);

//   // Set thresholds for all 12 electrodes:
//   // Touch thresholds start at 0x41, release at 0x42, then pairs per channel.
//   // We'll do touch=6, release=3
//   Serial.println("Writing thresholds...");
//   for (uint8_t i = 0; i < 12; i++) {
//     dumpAck("TTH", write8(0x41 + 2*i, 6));
//     dumpAck("RTH", write8(0x42 + 2*i, 3));
//   }

//   // Run mode: enable 12 electrodes, calibration lock OFF (CL=00), ELE=12 -> 0x0C
//   Serial.println("Run mode (ECR=0x0C)...");
//   dumpAck("ECR run", write8(0x5E, 0x0C));

//   delay(1500); // hands off; let baselines form

//   Serial.print("ECR readback = 0x"); Serial.println(read8(0x5E), HEX);
//   Serial.print("TTH0 readback= "); Serial.println(read8(0x41));
//   Serial.print("RTH0 readback= "); Serial.println(read8(0x42));
//   Serial.println();
// }

// void loop() {
//   Serial.println("ch, baseline, filtered, delta");
//   for (uint8_t i = 0; i < 12; i++) {
//     uint8_t  b = read8(0x1E + i);         // baseline is 8-bit
//     uint16_t f = read16(0x04 + 2*i);      // filtered is 16-bit
//     int16_t  d = (int16_t)f - (int16_t)((uint16_t)b * 8);
//     Serial.print(i); Serial.print(", ");
//     Serial.print(b); Serial.print(", ");
//     Serial.print(f); Serial.print(", ");
//     Serial.println(d);
//   }

//   uint16_t t = read16(0x00);
//   Serial.print("touch mask (raw 0x00..01) = 0b");
//   Serial.println(t, BIN);

//   Serial.println("---");
//   delay(300);
// }
