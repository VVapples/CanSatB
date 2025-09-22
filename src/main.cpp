#include <Arduino.h>
#include "motor.h"

/*
 * Simple Motor Control Test using motorWrite() function
 * 
 * WIRING INSTRUCTIONS:
 * 
 * AE-TB6612-BO Breakout Board Pins:
 * ================================
 * VCC    -> 3.3V or 5V (ESP32 3.3V pin)
 * GND    -> GND (ESP32 GND)
 * VM     -> Motor Power Supply (6V-15V) - Connect to external battery/power
 * VMGND  -> Motor Power Supply GND - Connect to external battery GND
 * 
 * Motor A Control:
 * AIN1   -> GPIO 12 (ESP32)
 * AIN2   -> GPIO 14 (ESP32)
 * PWMA   -> GPIO 27 (ESP32) - PWM capable pin
 * 
 * Motor B Control:
 * BIN1   -> GPIO 2 (ESP32)
 * BIN2   -> GPIO 4 (ESP32)
 * PWMB   -> GPIO 33 (ESP32) - PWM capable pin
 * 
 * Standby Control:
 * STBY   -> GPIO 15 (ESP32) - HIGH = Active, LOW = Standby
 * 
 * Motor Connections:
 * A01, A02 -> Motor A wires
 * B01, B02 -> Motor B wires
 * 
 * IMPORTANT: Connect VM and VMGND to external power supply (6V-15V)
 * The ESP32 3.3V is only for logic, not motor power!
 */

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("========================================");
  Serial.println("    Motor Control Test - motorWrite()  ");
  Serial.println("========================================");
  Serial.println();
  
  // Initialize motor driver
  setupMotors();
  
  Serial.println("Motor driver initialized successfully!");
  Serial.println("Starting simple motor tests in 3 seconds...");
  Serial.println("Usage: motorWrite('A', speed) or motorWrite('B', speed)");
  Serial.println("Speed range: -255 to 255 (negative = reverse)");
  Serial.println();
  delay(3000);
}

void loop() {
  Serial.println("=== Simple Motor Test Cycle ===");
  
  // Test Motor A with different speeds
  Serial.println("Testing Motor A:");
  Serial.println("  Forward at 50% speed");
  motorWrite('A', 128);    // 50% forward
  delay(2000);
  
  Serial.println("  Forward at 100% speed");
  motorWrite('A', 255);    // 100% forward
  delay(2000);
  
  Serial.println("  Reverse at 50% speed");
  motorWrite('A', -128);   // 50% reverse
  delay(2000);
  
  Serial.println("  Stop");
  motorWrite('A', 0);      // Stop
  delay(1000);
  
  // Test Motor B with different speeds
  Serial.println("Testing Motor B:");
  Serial.println("  Forward at 75% speed");
  motorWrite('B', 192);    // 75% forward
  delay(2000);
  
  Serial.println("  Reverse at 75% speed");
  motorWrite('B', -192);   // 75% reverse
  delay(2000);
  
  Serial.println("  Stop");
  motorWrite('B', 0);      // Stop
  delay(1000);
  
  // Test both motors together
  Serial.println("Testing Both Motors:");
  Serial.println("  Both forward at different speeds");
  motorWrite('A', 200);    // Motor A at ~78% forward
  motorWrite('B', 150);    // Motor B at ~59% forward
  delay(2000);
  
  Serial.println("  Turn left (A forward, B reverse)");
  motorWrite('A', 150);    // Motor A forward
  motorWrite('B', -150);   // Motor B reverse
  delay(2000);
  
  Serial.println("  Turn right (A reverse, B forward)");
  motorWrite('A', -150);   // Motor A reverse
  motorWrite('B', 150);    // Motor B forward
  delay(2000);
  
  Serial.println("  Stop both motors");
  motorWrite('A', 0);
  motorWrite('B', 0);
  
  Serial.println("=== Test Cycle Complete ===");
  Serial.println("Next cycle in 5 seconds...");
  Serial.println();
  delay(5000);
}
