#include <Arduino.h>
#include "gps.h"

bool gpsDetected = false;
uint32_t lastStatusCheck = 0;

void setup() {
  delay(2000); // Give some time for the Serial monitor to start
  Serial.begin(115200);
  Serial.println("=== Ultimate GPS Breakout v3 Test ===");
  Serial.println("Initializing GPS module...");

  setupGps(9, 10); // TX=9, RX=10 pins

  // Configure Ultimate GPS v3 for optimal performance
  Serial.println("Configuring Ultimate GPS v3 (MTK3339)...");
  configureGps(1, 9600); // 1 Hz update rate, 9600 baud
  
  Serial.println("Waiting for GPS data...");
  Serial.println("----------------------------------------");
}

void loop() {
  if (updateGps()) {
    gpsDetected = true;
    GpsData data = getGpsData();
    
    // Print comprehensive GPS information
    Serial.println("\n=== GPS UPDATE ===");
    
    if (data.hasFix) {
      Serial.printf("Position: %.6f°, %.6f°\n", data.latitude, data.longitude);
      Serial.printf("Altitude: %.2f m\n", data.altitude);
      Serial.printf("Speed: %.2f km/h\n", data.speed);
      Serial.printf("Course: %.1f°\n", data.course);
      Serial.printf("Satellites: %d\n", data.satelliteCount);
      Serial.printf("Fix Quality: %d (0=Invalid, 1=GPS, 2=DGPS)\n", data.fixQuality);
      Serial.printf("Fix Type: %d (1=No fix, 2=2D, 3=3D)\n", data.fixType);
      Serial.printf("HDOP: %.2f\n", data.hdop);
      
      if (data.timeValid) {
        // Extract time components
        uint32_t timeValue = data.time;
        uint8_t hour = (timeValue / 1000000) % 100;
        uint8_t minute = (timeValue / 10000) % 100;
        uint8_t second = (timeValue / 100) % 100;
        
        // Extract date components
        uint32_t dateValue = data.date;
        uint8_t day = (dateValue / 10000) % 100;
        uint8_t month = (dateValue / 100) % 100;
        uint8_t year = dateValue % 100;
        
        Serial.printf("Time: %02d:%02d:%02d UTC\n", hour, minute, second);
        Serial.printf("Date: %02d/%02d/20%02d\n", day, month, year);
      } else {
        Serial.println("Time: Not valid");
      }
    } else {
      Serial.println("Status: No GPS fix");
      Serial.printf("Satellites visible: %d\n", data.satelliteCount);
      Serial.println("Searching for satellites...");
    }
    
    Serial.println("----------------------------------------");
  } else {
    // Check module status periodically
    if (millis() - lastStatusCheck > 5000) { // Every 5 seconds
      lastStatusCheck = millis();
      
      if (!isGpsModuleDetected()) {
        Serial.println("WARNING: GPS module not detected!");
        Serial.println("Check wiring and power connections.");
      } else if (!gpsDetected) {
        Serial.println("GPS module detected, waiting for satellite fix...");
        Serial.println("This may take 30-60 seconds outdoors with clear sky view.");
      }
    }
  }
  
  delay(1000); // Check for updates every second
}