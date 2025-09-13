#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

/**
 * @brief Initializes the motor driver pins and settings.
 * Call this once in setup() before using any motor functions.
 * @param leftMotorPin1 First control pin for left motor
 * @param leftMotorPin2 Second control pin for left motor
 * @param rightMotorPin1 First control pin for right motor
 * @param rightMotorPin2 Second control pin for right motor
 */
void setupMotors(int leftMotorPin1, int leftMotorPin2, int rightMotorPin1, int rightMotorPin2);

/**
 * @brief Turns the CanSat by a given amount of degrees.
 * Positive degrees turn clockwise, negative degrees turn counter-clockwise.
 * @param degrees The amount to turn in degrees (-360 to 360)
 */
void motorTurn(float degrees);

/**
 * @brief Moves the CanSat forward or backward for a given distance.
 * Positive values move forward, negative values move backward.
 * @param distance The distance to move (units depend on calibration)
 */
void motorMove(float distance);

/**
 * @brief Stops both motors immediately.
 */
void motorStop();

/**
 * @brief Sets the base motor speed (0-255).
 * This affects the speed used by motorTurn and motorMove functions.
 * @param speed Motor speed from 0 (stopped) to 255 (full speed)
 */
void setMotorSpeed(int speed);

/**
 * @brief Calibration function to adjust turn accuracy.
 * Modify this value to make turns more accurate.
 * @param calibrationValue Milliseconds per degree of turn
 */
void setTurnCalibration(float calibrationValue);

/**
 * @brief Calibration function to adjust movement distance accuracy.
 * Modify this value to make distance movements more accurate.
 * @param calibrationValue Milliseconds per unit distance
 */
void setMoveCalibration(float calibrationValue);

#endif // MOTOR_H
