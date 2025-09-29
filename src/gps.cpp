#include "gps.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include "sd_logger.h"

static HardwareSerial gpsSerial(1); // Use Serial1
static TinyGPSPlus gps;

// The structure that holds our latest GPS data
static GpsData currentGpsData;

// GPS module's default baud rate (Ultimate GPS v3 default)
static const uint32_t GPS_BAUD_RATE = 9600;

// Module detection variables
static uint32_t lastDataReceived = 0;
static bool moduleDetected = false;
static uint32_t detectionTimeout = 10000; // 10 seconds timeout

// Implementation of the setupGps function
bool setupGps(int txPin, int rxPin) {
  gpsSerial.begin(GPS_BAUD_RATE, SERIAL_8N1, rxPin, txPin);
  delay(1000); // Allow time for the GPS module to initialize
  lastDataReceived = millis();
  moduleDetected = false;
  writeLogHeaders("gps_data.csv", "timestamp,latitude,longitude,altitude,hasFix,moduleDetected,satelliteCount,hdop,fixQuality,fixType,speed,course,timeValid,date,time,lastUpdate");
  return true;
}

// MTK3339-specific configuration commands for Ultimate GPS v3
void configureGps(int updateRate, uint32_t baudRate) {
}

// Enhanced implementation of the updateGps function for Ultimate GPS v3
bool updateGps() {
  bool newData = false;
  
  // Read all available characters from the GPS serial port
  while (gpsSerial.available() > 0) {
    static int charCount = 0;
    char c = gpsSerial.read();
    charCount++;
    
    if (gps.encode(c)) {
      lastDataReceived = millis();
      moduleDetected = true;
      newData = true;
    }
    
    // Prevent watchdog by limiting characters per loop
    if (charCount > 100) {
        charCount = 0;
        break; // Exit loop and let other tasks run
    }
  }

  // Check for module timeout
  if (millis() - lastDataReceived > detectionTimeout) {
    moduleDetected = false;
  }
  
  currentGpsData.moduleDetected = moduleDetected;

  // TinyGPS++ updates its internal state with every character.
  // Check if location data has been updated
  if (gps.location.isUpdated() || gps.date.isUpdated() || gps.time.isUpdated()) {
    currentGpsData.lastUpdate = millis();
    
    // Location data
    if (gps.location.isValid()) {
      currentGpsData.hasFix = true;
      currentGpsData.latitude = gps.location.lat();
      currentGpsData.longitude = gps.location.lng();
    } else {
      currentGpsData.hasFix = false;
    }
    
    // Altitude data
    if (gps.altitude.isValid()) {
      currentGpsData.altitude = gps.altitude.meters();
    }
    
    // Satellite and quality data
    if (gps.satellites.isValid()) {
      currentGpsData.satelliteCount = gps.satellites.value();
    }
    
    // Dilution of Precision data
    if (gps.hdop.isValid()) {
      currentGpsData.hdop = gps.hdop.hdop();
    }
    
    // Speed and course data (Ultimate GPS v3 provides this)
    if (gps.speed.isValid()) {
      currentGpsData.speed = gps.speed.kmph(); // Speed in km/h
    }
    
    if (gps.course.isValid()) {
      currentGpsData.course = gps.course.deg(); // Course in degrees
    }
    
    // Time and date data
    if (gps.date.isValid() && gps.time.isValid()) {
      currentGpsData.timeValid = true;
      currentGpsData.date = gps.date.value(); // Format: ddmmyy
      currentGpsData.time = gps.time.value(); // Format: hhmmsscc
    } else {
      currentGpsData.timeValid = false;
    }
    
    // Fix quality information (extracted from internal TinyGPS++ data)
    // Note: TinyGPS++ doesn't directly expose fix quality, but we can infer it
    if (gps.location.isValid()) {
      currentGpsData.fixQuality = 1; // GPS fix (assume standard GPS)
      currentGpsData.fixType = 3;    // 3D fix (assume 3D if we have location + altitude)
    } else {
      currentGpsData.fixQuality = 0; // Invalid
      currentGpsData.fixType = 1;    // No fix
    }
    
    //logging
    String gpswriteBuffer = String(millis()) + "," +
                   String(currentGpsData.latitude, 6) + "," +
                   String(currentGpsData.longitude, 6) + "," +
                   String(currentGpsData.altitude, 2) + "," +
                   String(currentGpsData.hasFix ? 1 : 0) + "," +
                   String(currentGpsData.moduleDetected ? 1 : 0) + "," +
                   String(currentGpsData.satelliteCount) + "," +
                   String(currentGpsData.hdop, 2) + "," +
                   String(currentGpsData.fixQuality) + "," +
                   String(currentGpsData.fixType) + "," +
                   String(currentGpsData.speed, 2) + "," +
                   String(currentGpsData.course, 2) + "," +
                   String(currentGpsData.timeValid ? 1 : 0) + "," +
                   String(currentGpsData.date) + "," +
                   String(currentGpsData.time) + "," +
                   String(currentGpsData.lastUpdate);
  writeToLog("gps_data.csv", gpswriteBuffer);

    return true; // New data was processed
  }

  return false; // No new location data
}

// Implementation of the getGpsData function
GpsData getGpsData() {
  // Simply return the latest data we have stored.
  return currentGpsData;
}

// Implementation of the isGpsModuleDetected function
bool isGpsModuleDetected() {
  return moduleDetected && (millis() - lastDataReceived < detectionTimeout);
}

// Optimized GPS initialization for independent power GPS modules
bool initializeGpsWithIndependentPower(int txPin, int rxPin, int updateRate, uint32_t baudRate) {
  return true;
}