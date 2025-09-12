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

/**
 * @brief Initializes the serial communication with the GPS module.
 * Enhanced for Ultimate GPS Breakout v3 with MTK3339 chipset.
 * @param txPin The microcontroller pin that sends data TO the GPS RX pin.
 * @param rxPin The microcontroller pin that receives data FROM the GPS TX pin.
 */
void setupGps(int txPin, int rxPin);

/**
 * @brief Sends MTK3339-specific configuration commands to the Ultimate GPS v3.
 * @param updateRate Update rate in Hz (1-10 Hz, default 1)
 * @param baudRate Serial baud rate (4800, 9600, 19200, 38400, 57600, 115200)
 */
void configureGps(int updateRate = 1, uint32_t baudRate = 9600);

/**
 * @brief Reads data from the GPS module and parses it.
 * Enhanced to detect module presence and extract additional data fields.
 * @note This function should be called in every iteration of the main loop().
 * @return Returns true if new, updated location data was received, false otherwise.
 */
bool updateGps();

/**
 * @brief Gets the most recently parsed GPS data.
 * @return A GpsData struct containing the latest information from Ultimate GPS v3.
 */
GpsData getGpsData();

/**
 * @brief Checks if the GPS module is detected and responding.
 * @return True if module is responding with NMEA data, false otherwise.
 */
bool isGpsModuleDetected();

#endif // GPS_H