#include "gps.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>

// The serial connection to the GPS module
// We use static to keep these variables private to this file.
static HardwareSerial* gpsSerial = nullptr; // Will be initialized in setupGps

// The TinyGPS++ object that parses GPS data
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
void setupGps(int txPin, int rxPin) {
  // For ESP32, use HardwareSerial. SERIAL_8N1 is the default config.
  int serialnum = 1; // Default to Serial1 (Pin 9/10)
  // Check pin combinations to determine which HardwareSerial to use
  if ((txPin == 1 && rxPin == 3) || (txPin == 3 && rxPin == 1)) {
    serialnum = 0; // Serial0 (USB serial)
  } else if ((txPin == 17 && rxPin == 16) || (txPin == 16 && rxPin == 17)) {
    serialnum = 2; // Serial2
  }
  // Serial1 uses pins 9/10 by default, so keep serialnum = 1 for other combinations
  gpsSerial = new HardwareSerial(serialnum);
  gpsSerial->begin(GPS_BAUD_RATE, SERIAL_8N1, rxPin, txPin);
  
  // Initialize GPS data structure with default values
  currentGpsData = {0}; // Zero-initialize all fields
  currentGpsData.moduleDetected = false;
  currentGpsData.hasFix = false;
  currentGpsData.timeValid = false;
  
  // Wait a moment for GPS module to initialize
  delay(1000);
}

// MTK3339-specific configuration commands for Ultimate GPS v3
void configureGps(int updateRate, uint32_t baudRate) {
  if (gpsSerial == nullptr) return;
  
  // MTK3339 command to set update rate (1-10 Hz)
  if (updateRate >= 1 && updateRate <= 10) {
    uint16_t period = 1000 / updateRate; // Convert Hz to milliseconds
    char rateCmd[50];
    sprintf(rateCmd, "$PMTK220,%d*", period);
    
    // Calculate checksum
    uint8_t checksum = 0;
    for (int i = 1; i < strlen(rateCmd) - 1; i++) {
      checksum ^= rateCmd[i];
    }
    sprintf(rateCmd + strlen(rateCmd) - 1, "%02X\r\n", checksum);
    
    gpsSerial->print(rateCmd);
    delay(100);
  }
  
  // MTK3339 command to set baud rate (if different from current)
  if (baudRate != GPS_BAUD_RATE) {
    char baudCmd[50];
    sprintf(baudCmd, "$PMTK251,%lu*", baudRate);
    
    // Calculate checksum
    uint8_t checksum = 0;
    for (int i = 1; i < strlen(baudCmd) - 1; i++) {
      checksum ^= baudCmd[i];
    }
    sprintf(baudCmd + strlen(baudCmd) - 1, "%02X\r\n", checksum);
    
    gpsSerial->print(baudCmd);
    delay(100);
    
    // Restart serial with new baud rate
    gpsSerial->end();
    gpsSerial->begin(baudRate, SERIAL_8N1);
  }
  
  // Enable specific NMEA sentences for optimal data (RMC + GGA + GSA + GSV)
  gpsSerial->print("$PMTK314,0,1,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n");
  delay(100);
}
// Enhanced implementation of the updateGps function for Ultimate GPS v3
bool updateGps() {
  bool newData = false;
  
  // Read all available characters from the GPS serial port
  while (gpsSerial->available() > 0) {
    char c = gpsSerial->read();
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