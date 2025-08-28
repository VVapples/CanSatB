#include "ultrasonic.h"

// Private (static) variables to store the pin numbers
static int trigPin_local;
static int echoPin_local;

void setupUltrasonic(int trigPin, int echoPin) {
  trigPin_local = trigPin;
  echoPin_local = echoPin;
  pinMode(trigPin_local, OUTPUT);
  pinMode(echoPin_local, INPUT);
}

float getDistanceCm() {
  // 1. Send the Trigger Pulse
  // Ensure the trigger pin is low first
  digitalWrite(trigPin_local, LOW);
  delayMicroseconds(2);
  // Send a 10-microsecond high pulse to trigger the sensor
  digitalWrite(trigPin_local, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin_local, LOW);

  // 2. Read the Echo Pulse
  // The pulseIn() function measures the time (in microseconds) that the echo pin is HIGH.
  // We add a timeout of 30000 µs (approx. 5 meters range) to prevent it from getting stuck.
  long duration_us = pulseIn(echoPin_local, HIGH, 30000);

  // 3. Calculate the Distance
  // The speed of sound is approx. 343 m/s or 0.0343 cm/µs.
  // The sound travels to the object and back, so we divide the total time by 2.
  // Formula: distance = (duration * speed_of_sound) / 2
  float distance = (duration_us * 0.0343) / 2.0;

  if (duration_us == 0) {
    return -1.0; // Return -1.0 to indicate a timeout (no echo received)
  }

  return distance;
}