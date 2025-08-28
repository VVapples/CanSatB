#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <Arduino.h>

/**
 * @brief Initializes the ultrasonic sensor's pins.
 * @param trigPin The pin connected to the sensor's "Trig" pin.
 * @param echoPin The pin connected to the sensor's "Echo" pin.
 */
void setupUltrasonic(int trigPin, int echoPin);

/**
 * @brief Performs a single distance measurement.
 * @note This is a blocking function and can pause your code for a few milliseconds.
 * @return The measured distance in centimeters. Returns -1.0 if the reading times out.
 */
float getDistanceCm();

#endif // ULTRASONIC_H