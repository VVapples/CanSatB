#include "gps.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include "sd_logger.h"

// The serial connection to the GPS module
// We use static to keep these variables private to this file.
static HardwareSerial gpsSerial(1); // Will be initialized in setupGps

// The TinyGPS++ object that parses GPS data
static TinyGPSPlus gps;

// The structure that holds our latest GPS data
static GpsData currentGpsData;

// GPS module's default baud rate
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
    char c = gpsSerial.read();
    if (gps.encode(c)) {
      lastDataReceived = millis();
      moduleDetected = true;
      newData = true;
    }
  }

  // Check for module timeout
  if (millis() - lastDataReceived > detectionTimeout) {
    moduleDetected = false;
  }
  
  currentGpsData.moduleDetected = moduleDetected;

  // Always update GPS data regardless of validity
  currentGpsData.lastUpdate = millis();
  
  // Location data - always assign
  currentGpsData.hasFix = gps.location.isValid();
  currentGpsData.latitude = gps.location.lat();
  currentGpsData.longitude = gps.location.lng();
  
  // Altitude data - always assign
  currentGpsData.altitude = gps.altitude.meters();
  
  // Satellite and quality data - always assign
  currentGpsData.satelliteCount = gps.satellites.value();
  
  // Dilution of Precision data - always assign
  currentGpsData.hdop = gps.hdop.hdop();
  
  // Speed and course data - always assign
  currentGpsData.speed = gps.speed.kmph(); // Speed in km/h
  currentGpsData.course = gps.course.deg(); // Course in degrees
  
  // Time and date data - always assign
  currentGpsData.timeValid = gps.date.isValid() && gps.time.isValid();
  currentGpsData.date = gps.date.value(); // Format: ddmmyy
  currentGpsData.time = gps.time.value(); // Format: hhmmsscc
  
  // Fix quality information - always assign
  currentGpsData.fixQuality = gps.location.isValid() ? 1 : 0;
  currentGpsData.fixType = gps.location.isValid() ? 3 : 1;
  
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

  return true; // Always return true since we always update data

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