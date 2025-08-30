#include <Arduino.h>
#include "sd_logger.h"

// SD card pin definitions
#define SD_CS_PIN 5  // Use GPIO 5 for CS pin (common on ESP32)

// Global variables
bool sdInitialized = false;
int logCounter = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(2000); // Give time for Serial Monitor to connect
  
  Serial.println("=== SD Card Logger Test ===");
  Serial.println("Initializing SD card...");
  
  // Initialize SD card
  sdInitialized = setupSdLogger(SD_CS_PIN);
  
  if (sdInitialized) {
    Serial.println("SD card initialized successfully!");
    Serial.println("Flight directory created.");
    
    // Write headers to CSV files using the new function
    writeLogHeaders("test_data.csv", "Counter,Timestamp,Random_Value");
    writeLogHeaders("sensor_log.csv", "Time,Temperature,Humidity,Pressure");
    
    Serial.println("CSV files initialized with column headers.");
  } else {
    Serial.println("ERROR: SD card initialization failed!");
    Serial.println("Check connections and SD card.");
  }
}

void loop() {
  if (sdInitialized) {
    // Write test data every 2 seconds
    logCounter++;
    
    // Create timestamp
    String timestamp = String(millis());
    
    // Generate some test data
    int randomValue = random(1, 100);
    float fakeTemp = 20.0 + random(-50, 150) / 10.0;  // 15.0 to 35.0°C
    float fakeHumidity = 40.0 + random(0, 600) / 10.0;  // 40.0 to 100.0%
    int fakePressure = 1000 + random(-50, 50);  // 950 to 1050 hPa
    
    // Write to test_data.csv
    String testData = String(logCounter) + "," + timestamp + "," + String(randomValue);
    writeToLog("test_data.csv", testData);
    
    // Write to sensor_log.csv
    String sensorData = timestamp + "," + String(fakeTemp, 1) + "," + 
                       String(fakeHumidity, 1) + "," + String(fakePressure);
    writeToLog("sensor_log.csv", sensorData);
    
    // Print status to Serial Monitor
    Serial.println("Data logged - Entry #" + String(logCounter));
    Serial.println("  Test data: " + testData);
    Serial.println("  Sensor data: " + sensorData);
    Serial.println("  Files updated successfully!");
    Serial.println();
    
    delay(2000); // Log data every 2 seconds
  } else {
    Serial.println("SD card not available - skipping log entry");
    delay(5000); // Wait longer if SD card failed
  }
}