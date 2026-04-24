// Simple WAV recorder for Teensy 4.1 - Mono input (A0 by default, change if needed)
// - 16-bit, 44.1 kHz
// - Writes to built-in microSD as "RECORD.WAV"

#include "main.h"
#include "dspHelpers.h"


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

AudioInputAnalog         adc1(AUDIO_INPUT_PIN);  // mono analog in
AudioRecordQueue         queue1;                 // queue for recording
AudioConnection          patchCord1(adc1, queue1);

// No audio shield, so no AudioControl object needed

// -------- SD / WAV Globals --------
const uint32_t SAMPLE_RATE      = 44100;
const uint16_t BITS_PER_SAMPLE  = 16;
const uint16_t NUM_CHANNELS     = 1;    // mono

bool isRecording = true;

uint16_t time_since_last_tune_ms = 1000;
uint64_t last_tune_time_ms = 1000;

s_yin_buffer sample_bank;
int64_t d[MAX_TAU];  // accumulator for the squared differences
float d_cmnd[MAX_TAU];    // normalised, float

std::string current_closest_note = "Poop";  

void initBuffer(s_yin_buffer *buff){
  for (int i = 0; i < YIN_BUFF_SIZE; i++)
  {
    buff->history[i] = 0;
  }
}

bool addToBuffer(s_yin_buffer *buff, int16_t new_val){
  buff->history[buff->last_index++] = new_val;

  if (buff->last_index == YIN_BUFF_SIZE) {
    buff->last_index = 0;
    return true;
  }
  else { 
    return false;
  }
}

void initDisplay(){
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(200); // Pause for 2 seconds

  return;
  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, WHITE);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(2000);

  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setTextWrap(false);
  display.setCursor(44, 34);
  display.println("C#3");

  display.display();

  delay(2000);
}

void updateDisplayTuning(int circlePos){
  
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setTextWrap(false);
  display.setCursor(44, 34);
  display.println(current_closest_note.c_str());

  display.drawCircle(circlePos, 60, 2, WHITE);

  display.display();

}

void printTauVals(){
  Serial.print("[");
  for (int i = 0; i < MAX_TAU - 1; i++)
  {
    Serial.print(d[i]);
    Serial.print(", ");
  }
  Serial.print(d[MAX_TAU - 1]);
  Serial.println("]");
  
}

float YIN(){
  // run YIN algorithm on sample_bank
  for (int tau = 1; tau < MAX_TAU; tau++) {
    d[tau] = 0;
    for (int i = 0; i < YIN_W; i++) {
        int32_t delta = (int32_t)sample_bank.history[i] - (int32_t)sample_bank.history[i + tau];
        d[tau] += delta * delta;
    }
  }

  d_cmnd[0] = 1.0f;
  float running_sum = 0.0f;

  for (int tau = 1; tau < MAX_TAU; tau++) {
    running_sum += (float)d[tau];
    d_cmnd[tau] = (float)d[tau] / (running_sum / (float)tau);
  }

  // Threshold pick — find first tau where d_cmnd dips below threshold
  const float THRESHOLD = 0.1f;
  int best_tau = -1;

  for (int tau = 2; tau < MAX_TAU; tau++) {
      if (d_cmnd[tau] < THRESHOLD) {
          best_tau = tau;
          break;
      }
  }

  // No pitch found — input is silence or unpitched noise
  if (best_tau == -1) {
      return -0.0;
  }

  // Parabolic interpolation — refine best_tau to sub-sample precision
  // fits a curve through the winning bin and its two neighbours
  float better_tau;
  if (best_tau > 0 && best_tau < MAX_TAU - 1) {
      float s0 = d_cmnd[best_tau - 1];
      float s1 = d_cmnd[best_tau];
      float s2 = d_cmnd[best_tau + 1];
      better_tau = best_tau + (s2 - s0) / (2.0f * (2.0f * s1 - s0 - s2));
  } else {
      better_tau = (float)best_tau;
  }

  // Convert tau to Hz
  float frequency = (float)SAMPLE_RATE / better_tau;

  // Serial.print(">tune:");
  // Serial.println(frequency);

	return frequency;
}

void printNoteToLED(Note note);

void testdrawstyles(void) {
  display.clearDisplay();

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Hello, world!"));

  display.setTextColor(BLACK, WHITE); // Draw 'inverse' text
  display.println(3.141592);

  display.setTextSize(2);             // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.print(F("0x")); display.println(0xDEADBEEF, HEX);

  display.display();
  delay(2000);
}

// -------- Setup --------
void setup() {

  initBuffer(&sample_bank);

  initDisplay();

  int start_time = millis();

  Serial.begin(115200);
  while (!Serial & millis() - start_time < 3000) {
    delay(100);
  }
  Serial.println("Teensy 4.1 WAV Recorder starting...");

  // Allocate audio memory blocks (tune as needed)
  AudioMemory(12);

  // Start recording
  queue1.begin();
  // isRecording = false;


  // testdrawstyles();

}

// -------- Main loop --------
void loop() {

  // TODO: Check for button press -> start recording, instead of starting immediately in setup().
  


  if (isRecording) {
    // Grab audio blocks from the queue and write them to SD
    while (queue1.available() > 0) {
      if (queue1.available() > 4) {
        queue1.freeBuffer();
      }
      // Each block from AudioRecordQueue is 128 samples of 16-bit data
      int16_t *buffer = (int16_t *)queue1.readBuffer();

      if (buffer) {
        // THIS IS ALWAYS 128 SAMPLES, BECAUSE THAT'S THE BLOCK SIZE OF AudioRecordQueue

        for (int i = 0; i < 128; i++)
        {
          if (addToBuffer(&sample_bank, buffer[i])){
            // Serial.println("TUNE!");
            float measured_freq = YIN();

            if (measured_freq){
              int x_dot_pos = getXValForLED(measured_freq, current_closest_note);
              updateDisplayTuning(x_dot_pos);
            }
						

						// call function in dspHelper here
						// printNoteToLED()
          }
        }
      }
      queue1.freeBuffer();
    }
  }
}
