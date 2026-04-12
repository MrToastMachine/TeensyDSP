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

// -------- Audio Objects --------
// Use analog input (mono). On Teensy 4.1, A0 is a good default.
// You can change AUDIO_INPUT_PIN below if you like.
#define AUDIO_INPUT_PIN A0

const int TUNE_INTERVAL_MS = 1000;

const int YIN_BUFF_SIZE = 2048;
const int MAX_TAU = 600;
const int YIN_W = YIN_BUFF_SIZE - MAX_TAU;

struct s_yin_buffer {
  int last_index = 0;
  int16_t history[YIN_BUFF_SIZE];
};
