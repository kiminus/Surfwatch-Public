#include <Arduino.h>

// Most AI-Thinker ESP32-CAM boards have a small LED on GPIO 4
// (sometimes called "onboard LED" or flash LED indicator)
#define LED_PIN 4

void setup() {
  Serial.begin(115200);
  delay(200);                   // Give serial some time to settle

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);   // LED off at start (most modules: LOW = on)

  Serial.println("\n=====================================");
  Serial.println("  ESP32-CAM Minimal Test - Build OK  ");
  Serial.println("=====================================");
  Serial.println("If you see this → upload & serial works!");
  Serial.println("Current time: " __DATE__ " " __TIME__);
  Serial.println("=====================================\n");
}

void loop() {
  // Simple blink
  digitalWrite(LED_PIN, HIGH);
  delay(400);
  digitalWrite(LED_PIN, LOW);
  delay(400);

  static int counter = 0;
  counter++;
  if (counter % 10 == 0) {
    Serial.printf("Running... count = %d\n", counter);
  }
}