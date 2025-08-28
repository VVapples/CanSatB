#include <Arduino.h>
#include "ultrasonic.h"

//pins SETLATER
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define GPS_COMMS_CONFIG SERIAL_8N1
#define ULTRASONIC_TRIGGER_PIN 18
#define ULTRASONIC_ECHO_PIN 19

void setup() {
  //Serial w/ pc
  Serial.begin(115200);
  Serial.println("Starting ultrasonic sensor test...");
  
  // Initialize ultrasonic sensor
  setupUltrasonic(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN);
  
  delay(1000); // Give sensor time to stabilize
}

void loop() {
  // Get distance measurement
  float distance = getDistanceCm();
  
  if (distance == -1.0) {
    Serial.println("Ultrasonic: No echo received (timeout)");
  } else {
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
  }

  delay(500); // Take readings every 500ms
}