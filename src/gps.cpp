#include "gps.h"
#include "sd_logger.h"
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
  // Initialize Serial0 for GPS debugging
  Serial.begin(115200);
  Serial.println("=== GPS Debug Mode Enabled ===");
  
  writeToLog("debug.csv", String(millis()) + ",GPS,SETUP_START,0,GPS Setup Starting - TX:" + String(txPin) + " RX:" + String(rxPin));
  Serial.println("GPS Setup Starting - TX:" + String(txPin) + " RX:" + String(rxPin));
  
  // For ESP32, use HardwareSerial. SERIAL_8N1 is the default config.
  int serialnum = 1; // Default to Serial1 (Pin 9/10)
  // Check pin combinations to determine which HardwareSerial to use
  if ((txPin == 1 && rxPin == 3) || (txPin == 3 && rxPin == 1)) {
    serialnum = 0; // Serial0 (USB serial)
  } else if ((txPin == 17 && rxPin == 16) || (txPin == 16 && rxPin == 17)) {
    serialnum = 2; // Serial2
  }
  writeToLog("debug.csv", String(millis()) + ",GPS,SERIAL_SELECT,0,Using Serial" + String(serialnum) + " for GPS");
  Serial.println("Using Serial" + String(serialnum) + " for GPS");
  
  // Serial1 uses pins 9/10 by default, so keep serialnum = 1 for other combinations
  gpsSerial = new HardwareSerial(serialnum);
  gpsSerial->begin(GPS_BAUD_RATE, SERIAL_8N1, rxPin, txPin);
  
  writeToLog("debug.csv", String(millis()) + ",GPS,SERIAL_INIT,0,GPS Serial initialized at " + String(GPS_BAUD_RATE) + " baud");
  Serial.println("GPS Serial initialized at " + String(GPS_BAUD_RATE) + " baud");
  
  // Initialize GPS data structure with default values
  currentGpsData = {0}; // Zero-initialize all fields
  currentGpsData.moduleDetected = false;
  currentGpsData.hasFix = false;
  currentGpsData.timeValid = false;
  
  // Wait a moment for GPS module to initialize
  delay(1000);
  writeToLog("debug.csv", String(millis()) + ",GPS,INIT_DELAY,0,GPS module init delay completed");
  
  // Initialize GPS data CSV
  String headers = "timestamp,valid,latitude,longitude,altitude,satellites,hdop,speed_kmh,course,fix_quality,fix_type,date,time,time_valid";
  writeLogHeaders("gps_data.csv", headers);
  writeToLog("debug.csv", String(millis()) + ",GPS,CSV_INIT,0,GPS data CSV initialized");
  
  writeToLog("debug.csv", String(millis()) + ",GPS,SETUP_SUCCESS,0,GPS setup completed successfully");
  Serial.println("GPS setup completed successfully");
  Serial.println("=== GPS Debug Ready - Watching for data ===");
}

// MTK3339-specific configuration commands for Ultimate GPS v3
void configureGps(int updateRate, uint32_t baudRate) {
  if (gpsSerial == nullptr) return;
  
  writeToLog("debug.csv", String(millis()) + ",GPS,CONFIG_START,0,Starting GPS configuration");
  
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
    writeToLog("debug.csv", String(millis()) + ",GPS,UPDATE_RATE,0,Set update rate to " + String(updateRate) + " Hz");
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
    writeToLog("debug.csv", String(millis()) + ",GPS,BAUD_CHANGE,0,Changed baud rate to " + String(baudRate));
  }
  
  // Enable specific NMEA sentences for optimal data (RMC + GGA + GSA + GSV)
  gpsSerial->print("$PMTK314,0,1,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n");
  delay(100);
  writeToLog("debug.csv", String(millis()) + ",GPS,NMEA_CONFIG,0,NMEA sentences configured");
}

// Enhanced implementation of the updateGps function for Ultimate GPS v3
bool updateGps() {
  if (!gpsSerial) {
    return false;
  }
  
  bool newData = false;
  
  // Read all available characters from the GPS serial port
  int charsThisUpdate = 0;
  while (gpsSerial->available() > 0) {
    char c = gpsSerial->read();
    charsThisUpdate++;
    
    // Print raw GPS characters to Serial for debugging
    Serial.print(c);
    
    if (gps.encode(c)) {
      lastDataReceived = millis();
      moduleDetected = true;
      newData = true;
      Serial.println("\n>>> NEW GPS SENTENCE PARSED <<<");
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
    
    // Log GPS data to CSV
    String gpsDataLine = String(millis()) + "," +
                       String(currentGpsData.hasFix ? "1" : "0") + "," +
                       String(currentGpsData.latitude, 6) + "," +
                       String(currentGpsData.longitude, 6) + "," +
                       String(currentGpsData.altitude, 2) + "," +
                       String(currentGpsData.satelliteCount) + "," +
                       String(currentGpsData.hdop, 2) + "," +
                       String(currentGpsData.speed, 2) + "," +
                       String(currentGpsData.course, 2) + "," +
                       String(currentGpsData.fixQuality) + "," +
                       String(currentGpsData.fixType) + "," +
                       String(currentGpsData.date) + "," +
                       String(currentGpsData.time) + "," +
                       String(currentGpsData.timeValid ? "1" : "0");
    
    writeToLog("gps_data.csv", gpsDataLine);
    
    // Print detailed GPS info to Serial for debugging
    Serial.println("\n==== GPS DATA UPDATE ====");
    Serial.println("Time: " + String(millis()) + "ms");
    Serial.println("Fix: " + String(currentGpsData.hasFix ? "YES" : "NO"));
    if (currentGpsData.hasFix) {
      Serial.println("Lat: " + String(currentGpsData.latitude, 6));
      Serial.println("Lon: " + String(currentGpsData.longitude, 6));
      Serial.println("Alt: " + String(currentGpsData.altitude, 2) + "m");
    }
    Serial.println("Satellites: " + String(currentGpsData.satelliteCount));
    Serial.println("HDOP: " + String(currentGpsData.hdop, 2));
    Serial.println("Speed: " + String(currentGpsData.speed, 1) + " km/h");
    Serial.println("Course: " + String(currentGpsData.course, 1) + "°");
    if (currentGpsData.timeValid) {
      Serial.println("Date: " + String(currentGpsData.date));
      Serial.println("Time: " + String(currentGpsData.time));
    }
    Serial.println("========================\n");
    
    // Periodic status logging
    static unsigned long lastStatusLog = 0;
    if (millis() - lastStatusLog >= 30000) { // Every 30 seconds
      lastStatusLog = millis();
      writeToLog("debug.csv", String(millis()) + ",GPS,STATUS,0,Fix:" + String(currentGpsData.hasFix ? "YES" : "NO") + 
                " Sats:" + String(currentGpsData.satelliteCount) + 
                " HDOP:" + String(currentGpsData.hdop, 1));
      
      // Print periodic status to Serial
      Serial.println("=== GPS STATUS (30s) ===");
      Serial.println("Module detected: " + String(moduleDetected ? "YES" : "NO"));
      Serial.println("Fix status: " + String(currentGpsData.hasFix ? "YES" : "NO"));
      Serial.println("Satellites: " + String(currentGpsData.satelliteCount));
      Serial.println("======================");
    }
    
    return true; // New data was processed
  }

  // Print character count periodically even when no fix
  static unsigned long lastCharCount = 0;
  if (millis() - lastCharCount >= 5000 && charsThisUpdate > 0) { // Every 5 seconds
    lastCharCount = millis();
    Serial.println("GPS chars this update: " + String(charsThisUpdate) + 
                  ", Total: " + String(gps.charsProcessed()) + 
                  ", Module: " + String(moduleDetected ? "OK" : "TIMEOUT"));
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