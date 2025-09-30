#include "motor.h"
#include "sd_logger.h"

// TB6612 Motor Driver Pin Variables (set by setupMotors)
static int AIN1_PIN;
static int AIN2_PIN;
static int BIN1_PIN;
static int BIN2_PIN;

// Motor direction reversal flags (if needed for wiring/configuration)
static bool mortorAReversed = false;
static bool motorBReversed = true;

//other variables
static int AMultiplier = 1; // Speed multiplier for Motor A
static int BMultiplier = 1; // Speed multiplier for Motor B

void setupMotors(int ain1, int ain2, int bin1, int bin2) {
  // Store pin assignments
  AIN1_PIN = ain1;
  AIN2_PIN = ain2;
  BIN1_PIN = bin1;
  BIN2_PIN = bin2;
  
  // Set all pins as outputs
  pinMode(AIN1_PIN, OUTPUT);
  pinMode(AIN2_PIN, OUTPUT);
  pinMode(BIN1_PIN, OUTPUT);
  pinMode(BIN2_PIN, OUTPUT);
  
  // Initialize to safe state
  stopAllMotors();
  writeLogHeaders("motor_raw.csv", "timestamp,motor,speed");
  writeLogHeaders("motor_cmd.csv", "timestamp,command,duration_ms");

  if (mortorAReversed) {
    AMultiplier = -1;
  }
  if (motorBReversed) {
    BMultiplier = -1;
  }

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
      digitalWrite(AIN1_PIN, LOW);
      digitalWrite(AIN2_PIN, LOW);
    } else if (forward) {
      // Motor A forward
      digitalWrite(AIN1_PIN, HIGH);
      digitalWrite(AIN2_PIN, LOW);
    } else {
      // Motor A reverse
      digitalWrite(AIN1_PIN, LOW);
      digitalWrite(AIN2_PIN, HIGH);
    }
    writeToLog("motor_raw.csv", String(millis()) + ",A," + String(speed));
  } 
  else if (motor == 'B' || motor == 'b') {
    // Motor B control
    if (speed == 0) {
      // Stop motor B
      digitalWrite(BIN1_PIN, LOW);
      digitalWrite(BIN2_PIN, LOW);
    } else if (forward) {
      // Motor B forward
      digitalWrite(BIN1_PIN, HIGH);
      digitalWrite(BIN2_PIN, LOW);
    } else {
      // Motor B reverse
      digitalWrite(BIN1_PIN, LOW);
      digitalWrite(BIN2_PIN, HIGH);
    }
    writeToLog("motor_raw.csv", String(millis()) + ",B," + String(speed));
  } 
  else {
    // Invalid motor parameter - do nothing
  }
}

void stopAllMotors() {
  motorWrite('A', 0);
  motorWrite('B', 0);
}

void motorControl(String command, int time_ms) {
  writeToLog("motor_cmd.csv", String(millis()) + "," + command + "," + String(time_ms));
  if (command == "stop") {
    stopAllMotors();
    return;
  } else if (command == "forward") {
    motorWrite('A', 1 * AMultiplier); // Motor A forward
    motorWrite('B', 1 * BMultiplier); // Motor B forward
  } else if (command == "backward") {
    motorWrite('A', -1 * AMultiplier); // Motor A backward
    motorWrite('B', -1 * BMultiplier); // Motor B backward
  } else if (command == "left") {
    motorWrite('A', -1 * AMultiplier); // Motor A backward
    motorWrite('B', 1 * BMultiplier);  // Motor B forward
  } else if (command == "right") {
    motorWrite('A', 1 * AMultiplier);  // Motor A forward
    motorWrite('B', -1 * BMultiplier); // Motor B backward
  } else {
    return;
    // Unknown command - do nothing
  }
  delay(time_ms);
  stopAllMotors();
}