#include <Arduino.h>
#include "gps.h"
// Commented out SD logger to isolate GPS issue
// #include "sd_logger.h"

// GPS only version to avoid memory corruption from SD operations
static bool sdCardAvailable = false;

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("=== CanSat System Starting ===");
  Serial.println("GPS ONLY MODE - SD card disabled to prevent memory corruption");
  
  yield();
  delay(500);
  
  Serial.println("Initializing GPS...");
  
  // Initialize GPS with proper error handling
  try {
    setupGps(9, 10); // TX=9, RX=10 (working pin order)
    Serial.println("GPS hardware initialized");
    
    yield();
    delay(200);
    
    Serial.println("Configuring GPS...");
    configureGps(1, 9600);
    Serial.println("GPS configuration complete");
    
  } catch (...) {
    Serial.println("ERROR: GPS initialization failed!");
  }
  
  Serial.println("Setup complete - entering main loop");
}

void loop() {
  // Feed watchdog regularly
  yield();
  
  static uint32_t lastStatus = 0;
  static bool gpsDetected = false;
  
  if (updateGps()) {
    gpsDetected = true;
    GpsData data = getGpsData();
    
    // Print GPS info to Serial
    Serial.println("\n=== GPS UPDATE ===");
    if (data.hasFix) {
      Serial.printf("Position: %.6f°, %.6f°\n", data.latitude, data.longitude);
      Serial.printf("Altitude: %.2f m\n", data.altitude);
      Serial.printf("Speed: %.2f km/h\n", data.speed);
      Serial.printf("Satellites: %d\n", data.satelliteCount);
      Serial.printf("HDOP: %.2f\n", data.hdop);
    } else {
      Serial.println("Status: No GPS fix");
      Serial.printf("Satellites visible: %d\n", data.satelliteCount);
    }
    Serial.println("----------------------------------------");
  }
  
  // Status check every 5 seconds
  if (millis() - lastStatus > 5000) {
    lastStatus = millis();
    Serial.println("System alive - " + String(millis()) + "ms");
    
    if (!isGpsModuleDetected()) {
      Serial.println("GPS module not detected");
    } else if (!gpsDetected) {
      Serial.println("GPS module detected, waiting for satellite fix...");
    }
  }
  
  delay(1000);
}
