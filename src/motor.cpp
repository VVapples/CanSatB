#include "motor.h"
#include <Arduino.h>

// Pin assignments for DRV8835 motor driver
static int leftMotor_pin1;
static int leftMotor_pin2;
static int rightMotor_pin1;
static int rightMotor_pin2;

// Motor settings
static int motorSpeed = 200; // Default speed (0-255)

// Calibration values - adjust these for accuracy
static float turnCalibration = 10.0;  // Milliseconds per degree of turn
static float moveCalibration = 100.0; // Milliseconds per unit distance

void setupMotors(int leftMotorPin1, int leftMotorPin2, int rightMotorPin1, int rightMotorPin2) {
    leftMotor_pin1 = leftMotorPin1;
    leftMotor_pin2 = leftMotorPin2;
    rightMotor_pin1 = rightMotorPin1;
    rightMotor_pin2 = rightMotorPin2;
    
    // Set all motor pins as outputs
    pinMode(leftMotor_pin1, OUTPUT);
    pinMode(leftMotor_pin2, OUTPUT);
    pinMode(rightMotor_pin1, OUTPUT);
    pinMode(rightMotor_pin2, OUTPUT);
    
    // Initialize motors to stopped state
    motorStop();
}

void motorTurn(float degrees) {
    // Calculate turn duration based on calibration
    unsigned long turnDuration = abs(degrees) * turnCalibration;
    
    if (degrees > 0) {
        // Turn clockwise: left motor forward, right motor backward
        analogWrite(leftMotor_pin1, motorSpeed);
        analogWrite(leftMotor_pin2, 0);
        analogWrite(rightMotor_pin1, 0);
        analogWrite(rightMotor_pin2, motorSpeed);
    } else if (degrees < 0) {
        // Turn counter-clockwise: left motor backward, right motor forward
        analogWrite(leftMotor_pin1, 0);
        analogWrite(leftMotor_pin2, motorSpeed);
        analogWrite(rightMotor_pin1, motorSpeed);
        analogWrite(rightMotor_pin2, 0);
    } else {
        // No turn needed
        return;
    }
    
    // Run motors for calculated duration
    delay(turnDuration);
    
    // Stop motors
    motorStop();
}

void motorMove(float distance) {
    // Calculate movement duration based on calibration
    unsigned long moveDuration = abs(distance) * moveCalibration;
    
    if (distance > 0) {
        // Move forward: both motors forward
        analogWrite(leftMotor_pin1, motorSpeed);
        analogWrite(leftMotor_pin2, 0);
        analogWrite(rightMotor_pin1, motorSpeed);
        analogWrite(rightMotor_pin2, 0);
    } else if (distance < 0) {
        // Move backward: both motors backward
        analogWrite(leftMotor_pin1, 0);
        analogWrite(leftMotor_pin2, motorSpeed);
        analogWrite(rightMotor_pin1, 0);
        analogWrite(rightMotor_pin2, motorSpeed);
    } else {
        // No movement needed
        return;
    }
    
    // Run motors for calculated duration
    delay(moveDuration);
    
    // Stop motors
    motorStop();
}

void motorStop() {
    // Stop both motors by setting all pins to LOW
    analogWrite(leftMotor_pin1, 0);
    analogWrite(leftMotor_pin2, 0);
    analogWrite(rightMotor_pin1, 0);
    analogWrite(rightMotor_pin2, 0);
}

void setMotorSpeed(int speed) {
    // Constrain speed to valid PWM range
    motorSpeed = constrain(speed, 0, 255);
}

void setTurnCalibration(float calibrationValue) {
    turnCalibration = calibrationValue;
}

void setMoveCalibration(float calibrationValue) {
    moveCalibration = calibrationValue;
}
