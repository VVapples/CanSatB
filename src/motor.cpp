#include "motor.h"

// TB6612 Motor Driver Pin Definitions
#define STBY    15    // Standby pin - LOW = standby, HIGH = active
#define PWMA    27    // Motor A PWM (speed control)
#define AIN1    12    // Motor A direction pin 1
#define AIN2    14    // Motor A direction pin 2
#define PWMB    33    // Motor B PWM (speed control)
#define BIN1    2     // Motor B direction pin 1
#define BIN2    4     // Motor B direction pin 2

void setupMotors() {
  // Set all pins as outputs
  pinMode(STBY, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  
  // Initialize to safe state
  stopAllMotors();
  exitStandby();  // Make sure we're not in standby
  
  Serial.println("TB6612 Motor Driver initialized");
  Serial.println("Pin assignments:");
  Serial.println("  STBY: " + String(STBY));
  Serial.println("  Motor A - PWM: " + String(PWMA) + ", IN1: " + String(AIN1) + ", IN2: " + String(AIN2));
  Serial.println("  Motor B - PWM: " + String(PWMB) + ", IN1: " + String(BIN1) + ", IN2: " + String(BIN2));
}

void motorWrite(char motor, int speed) {
  // Constrain speed to valid range (-255 to 255)
  speed = constrain(speed, -255, 255);
  
  // Get absolute speed for PWM
  int pwmSpeed = abs(speed);
  
  // Determine direction (positive = forward, negative = reverse)
  bool forward = (speed >= 0);
  
  if (motor == 'A' || motor == 'a') {
    // Motor A control
    if (speed == 0) {
      // Stop motor A
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, LOW);
      analogWrite(PWMA, 0);
      Serial.println("Motor A: STOPPED");
    } else if (forward) {
      // Motor A forward
      digitalWrite(AIN1, HIGH);
      digitalWrite(AIN2, LOW);
      analogWrite(PWMA, pwmSpeed);
      Serial.println("Motor A: FORWARD at " + String(pwmSpeed) + "/255 (" + String((pwmSpeed*100)/255) + "%)");
    } else {
      // Motor A reverse
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, HIGH);
      analogWrite(PWMA, pwmSpeed);
      Serial.println("Motor A: REVERSE at " + String(pwmSpeed) + "/255 (" + String((pwmSpeed*100)/255) + "%)");
    }
  } 
  else if (motor == 'B' || motor == 'b') {
    // Motor B control
    if (speed == 0) {
      // Stop motor B
      digitalWrite(BIN1, LOW);
      digitalWrite(BIN2, LOW);
      analogWrite(PWMB, 0);
      Serial.println("Motor B: STOPPED");
    } else if (forward) {
      // Motor B forward
      digitalWrite(BIN1, HIGH);
      digitalWrite(BIN2, LOW);
      analogWrite(PWMB, pwmSpeed);
      Serial.println("Motor B: FORWARD at " + String(pwmSpeed) + "/255 (" + String((pwmSpeed*100)/255) + "%)");
    } else {
      // Motor B reverse
      digitalWrite(BIN1, LOW);
      digitalWrite(BIN2, HIGH);
      analogWrite(PWMB, pwmSpeed);
      Serial.println("Motor B: REVERSE at " + String(pwmSpeed) + "/255 (" + String((pwmSpeed*100)/255) + "%)");
    }
  } 
  else {
    Serial.println("Error: Invalid motor '" + String(motor) + "'. Use 'A' or 'B'.");
  }
}

void stopAllMotors() {
  motorWrite('A', 0);
  motorWrite('B', 0);
}

void enterStandby() {
  digitalWrite(STBY, LOW);
  Serial.println("Motor driver in STANDBY mode (low power)");
}

void exitStandby() {
  digitalWrite(STBY, HIGH);
  Serial.println("Motor driver ACTIVE (ready for operation)");
}