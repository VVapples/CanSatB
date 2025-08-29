#ifndef GPS_H
#define GPS_H

#include <Arduino.h>

// This structure holds all the relevant GPS data.
// It's a clean way to pass GPS info around your program.
struct GpsData {
  // Core Data
  float latitude;      // Latitude in degrees
  float longitude;     // Longitude in degrees
  float altitude;      // Altitude in meters

  // Status Info
  bool hasFix;         // True if the location data is valid and reliable
  int satelliteCount;  // Number of satellites used for the fix

  // Quality Metric
  float hdop;          // Horizontal Dilution of Precision (a measure of accuracy, lower is better)
};

/**
 * @brief Initializes the serial communication with the GPS module.
 * @param txPin The microcontroller pin that sends data TO the GPS RX pin.
 * @param rxPin The microcontroller pin that receives data FROM the GPS TX pin.
 */
void setupGps(int txPin, int rxPin);

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

#endif // GPS_H