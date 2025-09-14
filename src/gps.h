#ifndef GPS_H
#define GPS_H

#include <Arduino.h>

// This structure holds all the relevant GPS data.
// It's a clean way to pass GPS info around your program.
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

/**
 * @brief Initializes the serial communication with the GPS module.
 * This function sets up the hardware serial connection and initializes the GPS data structure.
 * @param txPin The microcontroller pin that sends data TO the GPS RX pin.
 * @param rxPin The microcontroller pin that receives data FROM the GPS TX pin.
 */
void setupGps(int txPin, int rxPin);

/**
 * @brief Configures the GPS module with specific settings (MTK3339/Ultimate GPS v3).
 * This function sends configuration commands to set update rate, baud rate, and NMEA sentences.
 * @param updateRate GPS update rate in Hz (1-10 Hz supported).
 * @param baudRate Serial communication baud rate (default 9600).
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
 * @return A GpsData struct containing the latest information.
 */
GpsData getGpsData();

/**
 * @brief Checks if the GPS module is detected and responding.
 * @return Returns true if GPS module is detected and has sent data recently, false otherwise.
 */
bool isGpsModuleDetected();

#endif // GPS_H