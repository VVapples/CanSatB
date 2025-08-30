#include <Arduino.h>
#include "led.h"

//pins SETLATER
#define LED_PIN 32

void setup() {
  //Serial w/ pc
  Serial.begin(115200);
  
  // Initialize LED
  if (setupLed(LED_PIN)) {
    Serial.println("LED initialized successfully");
    
    // Test Morse code - spell "SOS" in Morse code
    // S = ...  O = ---  S = ...
    setMorseCode("... --- ...");
    
  } else {
    Serial.println("LED initialization failed");
  }
}

void loop() {
  // Update LED patterns
  updateLed();
  
  // Optional: Change pattern every 10 seconds for demo
  static unsigned long lastChange = 0;
  if (millis() - lastChange > 10000) {
    static int patternIndex = 0;
    
    switch(patternIndex) {
      case 0:
        setMorseCode("... --- ..."); // SOS
        Serial.println("Playing SOS");
        break;
      case 1:
        setMorseCode(".- -... -.-."); // ABC
        Serial.println("Playing ABC");
        break;
      case 2:
        setMorseCode(".... ."); // HE (Hello)
        Serial.println("Playing HE");
        break;
      case 3:
        setLedState("FAST"); // Back to fast blink
        Serial.println("Fast blink");
        break;
    }
    
    patternIndex = (patternIndex + 1) % 4;
    lastChange = millis();
  }
}
