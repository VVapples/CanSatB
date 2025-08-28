#ifndef NINE_AXIS_H
#define NINE_AXIS_H

#include <Arduino.h>

// This structure holds all the processed data from the BNO055.
struct BnoData {
  // Fused Orientation Data (Euler Angles)
  float heading; // Yaw: 0 to 360 degrees
  float roll;    // -90 to +90 degrees
  float pitch;   // -180 to +180 degrees

  // Fused Linear Acceleration (gravity is removed)
  float linearAccelX; // in m/s^2
  float linearAccelY;
  float linearAccelZ;

  // Calibration Status (0=uncalibrated, 3=fully calibrated)
  uint8_t sys_cal;
  uint8_t gyro_cal;
  uint8_t accel_cal;
  uint8_t mag_cal;
};

/**
 * @brief Initializes the BNO055 sensor. Call this once in setup().
 * @return true if the sensor was found and initialized, false otherwise.
 */
bool setupBno();

/**
 * @brief Reads all current data from the sensor and stores it internally.
 * Call this in every iteration of your main loop().
 */
void updateBnoData();

/**
 * @brief Gets the most recently read sensor data.
 * @return A BnoData struct containing the latest information.
 */
BnoData getBnoData();

/**
 * @brief Checks if the sensor is fully calibrated.
 * @return true only if all components report a calibration level of 3.
 */
bool isBnoCalibrated();

#endif // NINE_AXIS_H