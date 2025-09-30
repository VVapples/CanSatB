#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

/**
 * Simple Motor Control Library for TB6612 Motor Driver
 * 
 * Usage: motorWrite(motor, speed)
 * - motor: 'A' or 'B' 
 * - speed: -255 to 255 
 *   - Positive values = Forward direction
 *   - Negative values = Reverse direction  
 *   - 0 = Stop motor
 * 
 * Examples:
 * motorWrite('A', 200);   // Motor A forward at 200/255 speed
 * motorWrite('A', -150);  // Motor A reverse at 150/255 speed
 * motorWrite('B', 0);     // Motor B stop
 */

// Function declarations
void setupMotors(int ain1, int ain2, int bin1, int bin2);
void motorWrite(char motor, int speed);
void stopAllMotors();

#endif // MOTOR_H