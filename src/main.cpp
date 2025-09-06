#include <Arduino.h>


// DRV8835 Motor Driver Pin Definitions for ESP32 (IN/IN Mode)
#define MOTOR_A_IN1 12   // Motor A direction/speed pin 1 (AIN1 -> GPIO12) - PWM for speed
#define MOTOR_A_IN2 14   // Motor A direction/speed pin 2 (AIN2 -> GPIO14) - PWM for speed  
#define MOTOR_B_IN1 2    // Motor B direction/speed pin 1 (BIN1 -> GPIO2) - PWM for speed
#define MOTOR_B_IN2 4    // Motor B direction/speed pin 2 (BIN2 -> GPIO4) - PWM for speed

// Motor test parameters
#define TEST_SPEED_LOW 100    // Low speed (0-255)
#define TEST_SPEED_MED 150    // Medium speed (0-255)
#define TEST_SPEED_HIGH 200   // High speed (0-255)
#define TEST_DELAY 2000       // Delay between tests in milliseconds

// put function declarations here:
void setupMotors();
void testMotorA();
void testMotorB();
void testBothMotors();
void motorAForward(int speed);
void motorAReverse(int speed);
void motorBForward(int speed);
void motorBReverse(int speed);
void stopMotorA();
void stopMotorB();
void stopAllMotors();
void printMotorStatus(String motor, String direction, int speed);

void setup() {
  //Serial w/ pc
  Serial.begin(115200);
  
  // Wait for serial to initialize
  while (!Serial && millis() < 5000) {
    delay(100);
  }
  
  Serial.println("========================================");
  Serial.println("DRV8835 Motor Driver Test Program");
  Serial.println("========================================");
  
  // Initialize motors
  setupMotors();
  
  Serial.println("Setup complete. Starting motor tests in 3 seconds...");
  delay(3000);
}

void loop() {
  Serial.println("\n=== Starting Motor Test Cycle ===");
  
  // Test Motor A
  testMotorA();
  delay(1000);
  
  // Test Motor B
  testMotorB();
  delay(1000);
  
  // Test Both Motors
  testBothMotors();
  delay(1000);
  
  Serial.println("=== Test Cycle Complete ===");
  Serial.println("Waiting 5 seconds before next cycle...\n");
  delay(5000);
}

// Motor setup function
void setupMotors() {
  Serial.println("Initializing DRV8835 Motor Driver (IN/IN Mode)...");
  
  // Set all motor pins as outputs
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
  pinMode(MOTOR_B_IN1, OUTPUT);
  pinMode(MOTOR_B_IN2, OUTPUT);
  
  // Initialize all motors to stopped state
  stopAllMotors();
  
  Serial.println("Motor pins configured:");
  Serial.println("  Motor A - IN1: " + String(MOTOR_A_IN1) + ", IN2: " + String(MOTOR_A_IN2));
  Serial.println("  Motor B - IN1: " + String(MOTOR_B_IN1) + ", IN2: " + String(MOTOR_B_IN2));
  Serial.println("Motors initialized and stopped.");
}

// Test Motor A with different speeds and directions
void testMotorA() {
  Serial.println("\n--- Testing Motor A ---");
  
  // Forward direction tests
  Serial.println("Motor A Forward Tests:");
  motorAForward(TEST_SPEED_LOW);
  delay(TEST_DELAY);
  
  motorAForward(TEST_SPEED_MED);
  delay(TEST_DELAY);
  
  motorAForward(TEST_SPEED_HIGH);
  delay(TEST_DELAY);
  
  stopMotorA();
  delay(500);
  
  // Reverse direction tests
  Serial.println("Motor A Reverse Tests:");
  motorAReverse(TEST_SPEED_LOW);
  delay(TEST_DELAY);
  
  motorAReverse(TEST_SPEED_MED);
  delay(TEST_DELAY);
  
  motorAReverse(TEST_SPEED_HIGH);
  delay(TEST_DELAY);
  
  stopMotorA();
  Serial.println("Motor A test complete.");
}

// Test Motor B with different speeds and directions
void testMotorB() {
  Serial.println("\n--- Testing Motor B ---");
  
  // Forward direction tests
  Serial.println("Motor B Forward Tests:");
  motorBForward(TEST_SPEED_LOW);
  delay(TEST_DELAY);
  
  motorBForward(TEST_SPEED_MED);
  delay(TEST_DELAY);
  
  motorBForward(TEST_SPEED_HIGH);
  delay(TEST_DELAY);
  
  stopMotorB();
  delay(500);
  
  // Reverse direction tests
  Serial.println("Motor B Reverse Tests:");
  motorBReverse(TEST_SPEED_LOW);
  delay(TEST_DELAY);
  
  motorBReverse(TEST_SPEED_MED);
  delay(TEST_DELAY);
  
  motorBReverse(TEST_SPEED_HIGH);
  delay(TEST_DELAY);
  
  stopMotorB();
  Serial.println("Motor B test complete.");
}

// Test both motors simultaneously
void testBothMotors() {
  Serial.println("\n--- Testing Both Motors ---");
  
  // Both motors forward
  Serial.println("Both Motors Forward:");
  motorAForward(TEST_SPEED_MED);
  motorBForward(TEST_SPEED_MED);
  delay(TEST_DELAY);
  
  stopAllMotors();
  delay(500);
  
  // Both motors reverse
  Serial.println("Both Motors Reverse:");
  motorAReverse(TEST_SPEED_MED);
  motorBReverse(TEST_SPEED_MED);
  delay(TEST_DELAY);
  
  stopAllMotors();
  delay(500);
  
  // Opposite directions (turn test)
  Serial.println("Turn Test - Motor A Forward, Motor B Reverse:");
  motorAForward(TEST_SPEED_MED);
  motorBReverse(TEST_SPEED_MED);
  delay(TEST_DELAY);
  
  stopAllMotors();
  delay(500);
  
  Serial.println("Turn Test - Motor A Reverse, Motor B Forward:");
  motorAReverse(TEST_SPEED_MED);
  motorBForward(TEST_SPEED_MED);
  delay(TEST_DELAY);
  
  stopAllMotors();
  Serial.println("Both motors test complete.");
}

// Motor A control functions
void motorAForward(int speed) {
  // DRV8835 IN/IN Mode: Forward = IN1=PWM, IN2=LOW
  analogWrite(MOTOR_A_IN1, speed);
  digitalWrite(MOTOR_A_IN2, LOW);
  printMotorStatus("Motor A", "Forward", speed);
}

void motorAReverse(int speed) {
  // DRV8835 IN/IN Mode: Reverse = IN1=LOW, IN2=PWM
  digitalWrite(MOTOR_A_IN1, LOW);
  analogWrite(MOTOR_A_IN2, speed);
  printMotorStatus("Motor A", "Reverse", speed);
}

void stopMotorA() {
  // DRV8835 IN/IN Mode: Stop = IN1=LOW, IN2=LOW
  digitalWrite(MOTOR_A_IN1, LOW);
  digitalWrite(MOTOR_A_IN2, LOW);
  printMotorStatus("Motor A", "Stopped", 0);
}

// Motor B control functions
void motorBForward(int speed) {
  // DRV8835 IN/IN Mode: Forward = IN1=PWM, IN2=LOW
  analogWrite(MOTOR_B_IN1, speed);
  digitalWrite(MOTOR_B_IN2, LOW);
  printMotorStatus("Motor B", "Forward", speed);
}

void motorBReverse(int speed) {
  // DRV8835 IN/IN Mode: Reverse = IN1=LOW, IN2=PWM
  digitalWrite(MOTOR_B_IN1, LOW);
  analogWrite(MOTOR_B_IN2, speed);
  printMotorStatus("Motor B", "Reverse", speed);
}

void stopMotorB() {
  // DRV8835 IN/IN Mode: Stop = IN1=LOW, IN2=LOW
  digitalWrite(MOTOR_B_IN1, LOW);
  digitalWrite(MOTOR_B_IN2, LOW);
  printMotorStatus("Motor B", "Stopped", 0);
}

// Stop all motors
void stopAllMotors() {
  stopMotorA();
  stopMotorB();
  Serial.println("All motors stopped.");
}

// Print motor status to serial
void printMotorStatus(String motor, String direction, int speed) {
  Serial.println("  " + motor + " - " + direction + " at speed: " + String(speed) + "/255 (" + String((speed * 100) / 255) + "%)");
}