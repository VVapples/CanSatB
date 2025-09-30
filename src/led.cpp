#include <Arduino.h>
#include "led.h"
#include "sd_logger.h"

static int LED_PIN = -1; // Set to actual pin number if LED is used

void setupLed(int pin) {
  if (pin) {
    LED_PIN = pin;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW); // Turn off LED initially
    writeLogHeaders("led.csv", "timestamp,message");
    writeToLog("led.csv", String(millis()) + ",INIT_SUCCESS");
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
        writeToLog("led.csv", String(millis()) + ",ERROR_SIGNAL");
    } else if (message == "startup") {
        digitalWrite(LED_PIN, HIGH); // Solid ON
        writeToLog("led.csv", String(millis()) + ",INIT_SIGNAL");
    } else {
        dash();
        writeToLog("led.csv", String(millis()) + ",RUNNING_SIGNAL");
    }
}
