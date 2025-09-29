#include <Arduino.h>
#include "led.h"

static int LED_PIN = -1; // Set to actual pin number if LED is used

void setupLed(int pin) {
  if (pin) {
    LED_PIN = pin;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW); // Turn off LED initially
  }
}

void dot() {
    if (LED_PIN) {
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);
        delay(200);
    }
}

void dash() {
    if (LED_PIN) {
        digitalWrite(LED_PIN, HIGH);
        delay(600);
        digitalWrite(LED_PIN, LOW);
        delay(200);
    }
}

void ledMessage(String message) {
    if (message == "error") {
        dot();
    } else {
        dash();
    }
}
