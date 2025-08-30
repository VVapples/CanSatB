#include <Arduino.h>
#include "9axis.h"

// Pin definitions (MPU9250 uses I2C)
#define NINEAXIS_SDA_PIN 21
#define NINEAXIS_SCL_PIN 22

// Global variables
bool sensorReady = false;
int readingCount = 0;

// Simple calibration status indicators
bool isAccelCalibrated(Mpu9250Data data) {
  // Check if accelerometer magnitude is close to 1g (9.8 m/s²) when stationary
  float magnitude = sqrt(data.accelX*data.accelX + data.accelY*data.accelY + data.accelZ*data.accelZ);
  return (magnitude > 9.0 && magnitude < 10.5); // Allow some tolerance
}

bool isGyroCalibrated(Mpu9250Data data) {
  // Check if gyroscope readings are close to zero when stationary
  return (abs(data.gyroX) < 2.0 && abs(data.gyroY) < 2.0 && abs(data.gyroZ) < 2.0);
}

bool isMagCalibrated(Mpu9250Data data) {
  // Check if magnetometer has reasonable values (not all zeros)
  return (abs(data.magX) > 5.0 || abs(data.magY) > 5.0 || abs(data.magZ) > 5.0);
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(2000); // Give time for Serial Monitor to connect
  
  Serial.println("=== MPU9250 9-Axis Sensor Test ===");
  Serial.println("Initializing MPU9250...");
  
  // Initialize the 9-axis sensor
  sensorReady = setupMpu9250();
  
  if (sensorReady) {
    Serial.println("MPU9250 initialized successfully!");
    Serial.println("Starting sensor readings...");
    Serial.println();
    Serial.println("Calibration Tips:");
    Serial.println("- Keep sensor stationary for gyro calibration");
    Serial.println("- Rotate sensor in figure-8 pattern for mag calibration");
    Serial.println("- Accelerometer should read ~9.8 m/s² when stationary");
    Serial.println();
    
    // Print header for data columns
    Serial.println("Accel(m/s²)\t\tGyro(°/s)\t\tMag(µT)\t\t\tOrientation(°)\t\tTemp\tCalibration");
    Serial.println("X\tY\tZ\tX\tY\tZ\tX\tY\tZ\tPitch\tRoll\tHeading\t°C\tA|G|M");
    Serial.println("----------------------------------------------------------------------------------------");
  } else {
    Serial.println("ERROR: MPU9250 initialization failed!");
    Serial.println("Check connections:");
    Serial.println("- VCC → 3.3V");
    Serial.println("- GND → Ground");
    Serial.println("- SDA → GPIO 21");
    Serial.println("- SCL → GPIO 22");
  }
}

void loop() {
  if (sensorReady) {
    // Update sensor data
    updateMpu9250Data();
    
    // Get the latest data
    Mpu9250Data data = getMpu9250Data();
    
    readingCount++;
    
    // Print accelerometer data
    Serial.print(data.accelX, 2); Serial.print("\t");
    Serial.print(data.accelY, 2); Serial.print("\t");
    Serial.print(data.accelZ, 2); Serial.print("\t");
    
    // Print gyroscope data
    Serial.print(data.gyroX, 1); Serial.print("\t");
    Serial.print(data.gyroY, 1); Serial.print("\t");
    Serial.print(data.gyroZ, 1); Serial.print("\t");
    
    // Print magnetometer data
    Serial.print(data.magX, 1); Serial.print("\t");
    Serial.print(data.magY, 1); Serial.print("\t");
    Serial.print(data.magZ, 1); Serial.print("\t");
    
    // Print calculated orientation
    Serial.print(data.pitch, 1); Serial.print("\t");
    Serial.print(data.roll, 1); Serial.print("\t");
    Serial.print(data.heading, 1); Serial.print("\t");
    
    // Print temperature
    Serial.print(data.temperature, 1); Serial.print("\t");
    
    // Print calibration status
    Serial.print(isAccelCalibrated(data) ? "✓" : "✗");
    Serial.print("|");
    Serial.print(isGyroCalibrated(data) ? "✓" : "✗");
    Serial.print("|");
    Serial.print(isMagCalibrated(data) ? "✓" : "✗");
    
    // Print overall status every 10 readings
    if (readingCount % 10 == 0) {
      Serial.print(" - ");
      if (isAccelCalibrated(data) && isGyroCalibrated(data) && isMagCalibrated(data)) {
        Serial.print("ALL GOOD");
      } else {
        Serial.print("NEEDS CAL");
      }
    }
    
    Serial.println(); // New line
    
    delay(500); // Update every 500ms for readable output
  } else {
    Serial.println("Sensor not ready - retrying initialization...");
    sensorReady = setupMpu9250();
    delay(2000); // Wait 2 seconds before retrying
  }
}
