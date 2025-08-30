#include "led.h"

// LED configuration
static int ledPin = -1;
static bool ledInitialized = false;

// LED state management
static String currentState = "OFF";
static bool ledCurrentlyOn = false;
static unsigned long lastUpdateTime = 0;
static unsigned long onTime = 0;
static unsigned long offTime = 0;
static int patternStep = 0;
static bool patternDirection = true; // For pulse effect

// SOS pattern timing (dots=200ms, dashes=600ms, gaps=200ms)
static unsigned long sosPattern[] = {200, 200, 200, 200, 200, 600, 600, 200, 600, 200, 600, 600, 200, 200, 200, 200, 200, 1400}; // S-O-S + long gap
static int sosPatternLength = 18;

// Heartbeat pattern timing
static unsigned long heartbeatPattern[] = {100, 100, 100, 700}; // beat-beat-pause
static int heartbeatPatternLength = 4;

// Morse code variables
static String morsePattern = "";
static int morseIndex = 0;
static bool isMorseMode = false;
static unsigned long morseLastTime = 0;

bool setupLed(int pin) {
  // Validate pin number
  if (pin < 0 || pin > 39) {
    return false;
  }
  
  ledPin = pin;
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Start with LED off
  
  ledInitialized = true;
  currentState = "OFF";
  lastUpdateTime = millis();
  
  return true;
}

void setLedState(String state) {
  if (!ledInitialized) {
    return;
  }
  
  // Convert to uppercase for comparison
  state.toUpperCase();
  
  // Reset timing variables
  lastUpdateTime = millis();
  patternStep = 0;
  patternDirection = true;
  
  // Reset Morse code mode
  isMorseMode = false;
  morsePattern = "";
  morseIndex = 0;
  
  if (state == "OFF") {
    currentState = "OFF";
    digitalWrite(ledPin, LOW);
    ledCurrentlyOn = false;
  }
  else if (state == "ON") {
    currentState = "ON";
    digitalWrite(ledPin, HIGH);
    ledCurrentlyOn = true;
  }
  else if (state == "SLOW") {
    currentState = "SLOW";
    onTime = 1000;  // 1 second on
    offTime = 1000; // 1 second off
    digitalWrite(ledPin, HIGH);
    ledCurrentlyOn = true;
  }
  else if (state == "FAST") {
    currentState = "FAST";
    onTime = 200;   // 200ms on
    offTime = 200;  // 200ms off
    digitalWrite(ledPin, HIGH);
    ledCurrentlyOn = true;
  }
  else if (state == "PULSE") {
    currentState = "PULSE";
    onTime = 50;    // 50ms steps for pulse effect
    offTime = 50;
    digitalWrite(ledPin, HIGH);
    ledCurrentlyOn = true;
  }
  else if (state == "SOS") {
    currentState = "SOS";
    digitalWrite(ledPin, HIGH);
    ledCurrentlyOn = true;
  }
  else if (state == "HEARTBEAT") {
    currentState = "HEARTBEAT";
    digitalWrite(ledPin, HIGH);
    ledCurrentlyOn = true;
  }
  else if (state.startsWith("CUSTOM:")) {
    // Parse custom timing: CUSTOM:XXX:YYY
    int firstColon = state.indexOf(':', 7);
    int secondColon = state.indexOf(':', firstColon + 1);
    
    if (firstColon != -1 && secondColon != -1) {
      onTime = state.substring(7, firstColon).toInt();
      offTime = state.substring(firstColon + 1, secondColon).toInt();
      
      if (onTime > 0 && offTime > 0) {
        currentState = state;
        digitalWrite(ledPin, HIGH);
        ledCurrentlyOn = true;
      } else {
        return;
      }
    } else {
      return;
    }
  }
  else {
    return;
  }
}

void setMorseCode(String morseSequence) {
  if (!ledInitialized) {
    return;
  }
  
  // Reset all pattern variables
  lastUpdateTime = millis();
  patternStep = 0;
  patternDirection = true;
  
  // Set up Morse code mode
  morsePattern = morseSequence;
  morseIndex = 0;
  isMorseMode = true;
  currentState = "MORSE";
  
  // Start with LED off
  digitalWrite(ledPin, LOW);
  ledCurrentlyOn = false;
  morseLastTime = millis();
}

void updateLed() {
  if (!ledInitialized || currentState == "OFF" || currentState == "ON") {
    return; // No updates needed for static states
  }
  
  unsigned long currentTime = millis();
  unsigned long elapsed = currentTime - lastUpdateTime;
  
  if (currentState == "SLOW" || currentState == "FAST" || currentState.startsWith("CUSTOM:")) {
    // Simple on/off blinking
    if (ledCurrentlyOn && elapsed >= onTime) {
      digitalWrite(ledPin, LOW);
      ledCurrentlyOn = false;
      lastUpdateTime = currentTime;
    }
    else if (!ledCurrentlyOn && elapsed >= offTime) {
      digitalWrite(ledPin, HIGH);
      ledCurrentlyOn = true;
      lastUpdateTime = currentTime;
    }
  }
  else if (currentState == "PULSE") {
    // Pulse effect simulation using rapid on/off
    if (elapsed >= 50) { // Update every 50ms
      if (patternDirection) {
        patternStep++;
        if (patternStep >= 10) { // Fade to full brightness
          patternDirection = false;
        }
      } else {
        patternStep--;
        if (patternStep <= 0) { // Fade to off
          patternDirection = true;
        }
      }
      
      // Simple simulation: LED on for portion of the cycle
      bool shouldBeOn = (patternStep > 3);
      digitalWrite(ledPin, shouldBeOn ? HIGH : LOW);
      ledCurrentlyOn = shouldBeOn;
      lastUpdateTime = currentTime;
    }
  }
  else if (currentState == "SOS") {
    // SOS morse code pattern
    if (elapsed >= sosPattern[patternStep]) {
      ledCurrentlyOn = !ledCurrentlyOn;
      digitalWrite(ledPin, ledCurrentlyOn ? HIGH : LOW);
      
      patternStep++;
      if (patternStep >= sosPatternLength) {
        patternStep = 0; // Restart pattern
      }
      lastUpdateTime = currentTime;
    }
  }
  else if (currentState == "HEARTBEAT") {
    // Double pulse heartbeat pattern
    if (elapsed >= heartbeatPattern[patternStep]) {
      ledCurrentlyOn = !ledCurrentlyOn;
      digitalWrite(ledPin, ledCurrentlyOn ? HIGH : LOW);
      
      patternStep++;
      if (patternStep >= heartbeatPatternLength) {
        patternStep = 0; // Restart pattern
      }
      lastUpdateTime = currentTime;
    }
  }
  else if (currentState == "MORSE") {
    // Custom Morse code pattern
    unsigned long morseElapsed = currentTime - morseLastTime;
    
    if (morseIndex >= morsePattern.length()) {
      // Pattern finished, restart after a pause
      if (morseElapsed >= 2000) { // 2 second pause between repeats
        morseIndex = 0;
        digitalWrite(ledPin, LOW);
        ledCurrentlyOn = false;
        morseLastTime = currentTime;
      }
      return;
    }
    
    char currentChar = morsePattern.charAt(morseIndex);
    
    if (currentChar == '.') {
      // Dot: 200ms on, 200ms off
      if (!ledCurrentlyOn && morseElapsed >= 200) {
        // Finished the off period, turn on
        digitalWrite(ledPin, HIGH);
        ledCurrentlyOn = true;
        morseLastTime = currentTime;
      } else if (ledCurrentlyOn && morseElapsed >= 200) {
        // Finished the on period, turn off and move to next character
        digitalWrite(ledPin, LOW);
        ledCurrentlyOn = false;
        morseLastTime = currentTime;
        morseIndex++;
      }
    }
    else if (currentChar == '-') {
      // Dash: 600ms on, 200ms off
      if (!ledCurrentlyOn && morseElapsed >= 200) {
        // Finished the off period, turn on
        digitalWrite(ledPin, HIGH);
        ledCurrentlyOn = true;
        morseLastTime = currentTime;
      } else if (ledCurrentlyOn && morseElapsed >= 600) {
        // Finished the on period, turn off and move to next character
        digitalWrite(ledPin, LOW);
        ledCurrentlyOn = false;
        morseLastTime = currentTime;
        morseIndex++;
      }
    }
    else if (currentChar == ' ') {
      // Space: extra 400ms pause (total 600ms off between characters)
      if (morseElapsed >= 600) {
        morseIndex++;
        morseLastTime = currentTime;
      }
    }
    else {
      // Unknown character, skip it
      morseIndex++;
    }
  }
}

String getLedState() {
  return currentState;
}