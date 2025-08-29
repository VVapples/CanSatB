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

// GPS module's default baud rate
static const uint32_t GPS_BAUD_RATE = 9600;

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
  writeToLog("gps.txt", "GPS initialized on Serial" + String(serialnum));
}
// Implementation of the updateGps function
bool updateGps() {
  // Read all available characters from the GPS serial port
  while (gpsSerial->available() > 0) {
    gps.encode(gpsSerial->read());
  }

  // TinyGPS++ updates its internal state with every character.
  // We can check if a key piece of data, like location, has been
  // updated since the last time we checked.
  if (gps.location.isUpdated()) {
    // If location is valid, populate our struct with new data.
    if (gps.location.isValid()) {
      currentGpsData.hasFix = true;
      currentGpsData.latitude = gps.location.lat();
      currentGpsData.longitude = gps.location.lng();
      currentGpsData.satelliteCount = gps.satellites.value();
      currentGpsData.hdop = gps.hdop.hdop();
      currentGpsData.altitude = gps.altitude.meters();
    }
    // If the location is invalid (e.g., lost signal), mark it as no fix.
    else {
      currentGpsData.hasFix = false;
    }
    // Return true to signal that new data is available for processing.
    return true;
  }

  // If no new location data was found, return false.
  return false;
}

// Implementation of the getGpsData function
GpsData getGpsData() {
  // Simply return the latest data we have stored.
  return currentGpsData;
}