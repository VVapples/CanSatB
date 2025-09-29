#ifndef GPS_H
#define GPS_H

#include <Arduino.h>

// This structure holds all the relevant GPS data.
// Enhanced for Ultimate GPS Breakout v3 (MTK3339) capabilities
struct GpsData {
  // Core Data
  float latitude;      // Latitude in degrees
  float longitude;     // Longitude in degrees
  float altitude;      // Altitude in meters above MSL

  // Status Info
  bool hasFix;         // True if the location data is valid and reliable
  int satelliteCount;  // Number of satellites used for the fix
  int fixQuality;      // 0=invalid, 1=GPS fix, 2=DGPS fix
  int fixType;         // 1=no fix, 2=2D fix, 3=3D fix

  // Quality Metrics
  float hdop;          // Horizontal Dilution of Precision (lower is better)
  float pdop;          // Position Dilution of Precision
  float vdop;          // Vertical Dilution of Precision

  // Motion Data
  float speed;         // Speed over ground in km/h
  float course;        // Course over ground in degrees

  // Time Data
  uint32_t date;       // Date (ddmmyy)
  uint32_t time;       // Time (hhmmsscc)
  bool timeValid;      // True if time/date is valid

  // Module Status
  bool moduleDetected; // True if GPS module is responding
  uint32_t lastUpdate; // Timestamp of last successful data update
};

bool setupGps(int txPin, int rxPin);

bool updateGps();

GpsData getGpsData();

#endif // GPS_H