#include <Arduino.h>
#include "9axis.h"

// How often to print data to serial (in milliseconds)
#define PRINT_DELAY 500

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // wait for serial port to connect. Needed for native USB
  }
  Serial.println("## BNO055 Data Example ##");

  // Initialize the BNO055 sensor
  if (setupBno()) {
    Serial.println("BNO055 sensor initialized successfully!");
  } else {
    Serial.println("BNO055 sensor initialization failed. Check wiring.");
    // You might want to halt execution here if the sensor is critical
    while (1) {
      delay(1000);
    }
  }
}

void loop() {
  // Update the sensor data in every loop
  updateBnoData();

  // Get the latest data
  BnoData data = getBnoData();

  // Print the data
  Serial.print("Heading: ");
  Serial.print(data.heading);
  Serial.print(" | Roll: ");
  Serial.print(data.roll);
  Serial.print(" | Pitch: ");
  Serial.print(data.pitch);

  Serial.print(" || LinAccel X: ");
  Serial.print(data.linearAccelX);
  Serial.print(" Y: ");
  Serial.print(data.linearAccelY);
  Serial.print(" Z: ");
  Serial.println(data.linearAccelZ);

  // Wait a bit before printing again
  delay(PRINT_DELAY);
}