#ifndef GPS_H
#define GPS_H

#include <Arduino.h>

// This structure holds all the relevant GPS data.
// Enhanced for Ultimate GPS Breakout v3 (MTK3339) capabilities
struct GpsData {
  // Core Location Data
  float latitude;      // Latitude in degrees
  float longitude;     // Longitude in degrees
  float altitude;      // Altitude in meters

  // Status Info
  bool hasFix;         // True if the location data is valid and reliable
  bool moduleDetected; // True if GPS module is detected and responding
  int satelliteCount;  // Number of satellites used for the fix

  // Quality Metrics
  float hdop;          // Horizontal Dilution of Precision (lower is better)
  int fixQuality;      // Fix quality: 0=invalid, 1=GPS fix, 2=DGPS fix
  int fixType;         // Fix type: 1=no fix, 2=2D fix, 3=3D fix

  // Motion Data
  float speed;         // Speed in km/h
  float course;        // Course/heading in degrees

  // Time Data
  bool timeValid;      // True if date/time data is valid
  uint32_t date;       // Date in ddmmyy format
  uint32_t time;       // Time in hhmmsscc format
  
  // System Data
  uint32_t lastUpdate; // Timestamp of last data update (millis())
};

bool setupGps(int txPin, int rxPin);

bool updateGps();

GpsData getGpsData();

#endif // GPS_H