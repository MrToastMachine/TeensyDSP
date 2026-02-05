// #include <Arduino.h>
#include <Wire.h>
#include "Adafruit_MPR121.h"

Adafruit_MPR121 cap;

static const uint8_t NCH = 12;

// Software baseline in "filtered units"
float baseline[NCH];

// Touch state
bool isTouched[NCH];

// Hysteresis thresholds in filtered-units delta
// delta = baseline - filtered  (touch => delta increases)
float touchTh = 8.0f;
float releaseTh = 4.0f;

// Baseline adaptation:
// smaller = slower baseline (more stable, less drift)
float alpha = 0.01f; // 1% per sample (~slow). Increase if baseline never tracks drift.

uint32_t lastPrint = 0;

void setup() {
  Serial.begin(115200);
  delay(200);
  Wire.begin();

  if (!cap.begin(0x5B)) {
    Serial.println("MPR121 not found");
    while (1) delay(10);
  }

  // Initial baseline capture (hands off!)
  delay(500);
  for (uint8_t i = 0; i < NCH; i++) {
    baseline[i] = (float)cap.filteredData(i);
    isTouched[i] = false;
  }

  Serial.println("Software-touch mode (using filteredData only)");
  Serial.println("delta = baseline - filtered (touch makes delta grow)");
  Serial.println("Commands: th <touch> <release> | a <alpha>");
}

void loop() {
  // Optional simple serial commands to tune without recompiling
  if (Serial.available()) {
    String s = Serial.readStringUntil('\n');
    s.trim();
    if (s.startsWith("th ")) {
      float t, r;
      if (sscanf(s.c_str(), "th %f %f", &t, &r) == 2) {
        touchTh = t;
        releaseTh = r;
        if (releaseTh >= touchTh) releaseTh = touchTh * 0.5f;
        Serial.print("touchTh="); Serial.print(touchTh);
        Serial.print(" releaseTh="); Serial.println(releaseTh);
      }
    } else if (s.startsWith("a ")) {
      float a;
      if (sscanf(s.c_str(), "a %f", &a) == 1) {
        if (a < 0.0001f) a = 0.0001f;
        if (a > 0.2f) a = 0.2f;
        alpha = a;
        Serial.print("alpha="); Serial.println(alpha, 6);
      }
    }
  }

  for (uint8_t i = 0; i < NCH; i++) {
    float f = (float)cap.filteredData(i);
    float d = baseline[i] - f; // touch => f drops => d increases

    // Touch/release logic with hysteresis
    if (!isTouched[i]) {
      if (d >= touchTh) {
        isTouched[i] = true;
        Serial.print(i); Serial.println(" touched");
      } else {
        // Only update baseline when NOT touched
        baseline[i] = (1.0f - alpha) * baseline[i] + alpha * f;
      }
    } else {
      if (d <= releaseTh) {
        isTouched[i] = false;
        Serial.print(i); Serial.println(" released");
        // After release, re-seed baseline a bit toward current
        baseline[i] = (1.0f - 5*alpha) * baseline[i] + (5*alpha) * f;
      }
      // While touched, do NOT adapt baseline (prevents “learning the touch”)
    }
  }

  // Periodic debug: show deltas so you can choose thresholds
  if (millis() - lastPrint > 500) {
    lastPrint = millis();
    Serial.print("deltas: ");
    for (uint8_t i = 0; i < NCH; i++) {
      float f = (float)cap.filteredData(i);
      float d = baseline[i] - f;
      Serial.print((int)d);
      if (i != NCH - 1) Serial.print(", ");
    }
    Serial.println();
  }

  delay(10);
}
