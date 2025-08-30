#ifndef NINE_AXIS_H
#define NINE_AXIS_H

#include <Arduino.h>

// This structure holds all the processed data from the MPU9250.
struct Mpu9250Data {
  // Raw Accelerometer Data (m/s^2)
  float accelX;
  float accelY; 
  float accelZ;

  // Raw Gyroscope Data (degrees/second)
  float gyroX;
  float gyroY;
  float gyroZ;

  // Raw Magnetometer Data (uT - microTesla)
  float magX;
  float magY;
  float magZ;

  // Temperature (Celsius)
  float temperature;

  // Calculated orientation (simple estimation)
  float pitch;    // calculated from accelerometer
  float roll;     // calculated from accelerometer
  float heading;  // calculated from magnetometer
};

/**
 * @brief Initializes the MPU9250 sensor. Call this once in setup().
 * @return true if the sensor was found and initialized, false otherwise.
 */
bool setupMpu9250();

/**
 * @brief Reads all current data from the sensor and stores it internally.
 * Call this in every iteration of your main loop().
 */
void updateMpu9250Data();

/**
 * @brief Gets the most recently read sensor data.
 * @return A Mpu9250Data struct containing the latest information.
 */
Mpu9250Data getMpu9250Data();

/**
 * @brief Checks if the sensor is responding and providing valid data.
 * @return true if sensor is working properly.
 */
bool isMpu9250Ready();

#endif // NINE_AXIS_H