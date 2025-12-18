// /*********************************************************
// This is a library for the MPR121 12-channel Capacitive touch sensor

// Designed specifically to work with the MPR121 Breakout in the Adafruit shop 
//   ----> https://www.adafruit.com/products/

// These sensors use I2C communicate, at least 2 pins are required 
// to interface

// Adafruit invests time and resources providing this open source code, 
// please support Adafruit and open-source hardware by purchasing 
// products from Adafruit!

// Written by Limor Fried/Ladyada for Adafruit Industries.  
// BSD license, all text above must be included in any redistribution
// **********************************************************/

// /* This code is an example for dealing with the controls of the Cap Touch board I bought */

// #include <Wire.h>
// #include <Arduino.h>
// #include "Adafruit_MPR121.h"


// extern unsigned long lastSoundTime;

// #ifndef _BV
// #define _BV(bit) (1 << (bit)) 
// #endif

// // You can have up to 4 on one i2c bus but one is enough for testing!
// Adafruit_MPR121 cap = Adafruit_MPR121();

// // Keeps track of the last pins touched
// // so we know when buttons are 'released'
// uint16_t lasttouched = 0;
// uint16_t currtouched = 0;


// void setup() {
//   Serial.begin(9600);

//   while (!Serial) { // needed to keep leonardo/micro from starting too fast!
//     delay(10);
//   }
  
// }

// void loop() {
//   // Get the currently touched pads
//   currtouched = cap.touched();
  
//   for (uint8_t i=0; i<12; i++) {
//     // it if *is* touched and *wasnt* touched before, alert!
//     if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
//       Serial.print(i); Serial.println(" touched");
//     }
//     // if it *was* touched and now *isnt*, alert!
//     if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
//       Serial.print(i); Serial.println(" released");
//     }
//   }
// }


// /**/