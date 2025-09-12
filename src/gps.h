#ifndef GPS_H
#define GPS_H

#include <Arduino.h>

// Simplified GPS data structure for Ultimate GPS Breakout v3 (MTK3339 chipset)
struct GpsData {
  // Core Position Data
  double latitude;         // Latitude in degrees (high precision)
  double longitude;        // Longitude in degrees (high precision)
  float altitude;          // Altitude above MSL in meters
  
  // Status Information
  bool hasFix;            // True if GPS has a valid fix
  bool moduleDetected;    // True if GPS module is responding
  uint8_t satelliteCount; // Number of satellites in use
  uint8_t fixQuality;     // 0=Invalid, 1=GPS fix, 2=DGPS fix
  uint8_t fixType;        // 1=No fix, 2=2D fix, 3=3D fix
  
  // Accuracy Metrics
  float hdop;             // Horizontal Dilution of Precision
  
  // Motion Data
  float speed;            // Speed over ground in km/h
  float course;           // Course over ground in degrees
  
  // Time and Date (simplified format)
  uint32_t date;          // Date in format ddmmyy
  uint32_t time;          // Time in format hhmmsscc
  bool timeValid;         // True if time/date is valid
  
  // Module Health
  uint32_t lastUpdate;    // Timestamp of last successful update
};

/**
 * @brief Initializes the serial communication with the Ultimate GPS Breakout v3.
 * Sets up communication with MTK3339 chipset.
 * @param txPin The microcontroller pin that sends data TO the GPS RX pin.
 * @param rxPin The microcontroller pin that receives data FROM the GPS TX pin.
 */
void setupGps(int txPin, int rxPin);

/**
 * @brief Configure Ultimate GPS v3 specific settings (MTK3339 commands).
 * @param updateRate Update rate in Hz (1-10 Hz)
 * @param baudRate Serial baud rate to use
 */
void configureGps(int updateRate, uint32_t baudRate);

/**
 * @brief Reads data from the GPS module and parses it.
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
 * @brief Check if the Ultimate GPS v3 module is detected and responding.
 * @return true if module is detected and sending NMEA data, false otherwise.
 */
bool isGpsModuleDetected();

#endif // GPS_H