#ifndef NINE_AXIS_H
#define NINE_AXIS_H

#include <Arduino.h>

// This structure holds all the processed data from the BNO055.
struct Bno055Data {
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

  // Orientation from BNO055 fusion algorithm (degrees)
  float pitch;    // calculated orientation
  float roll;     // calculated orientation
  float heading;  // calculated orientation (yaw)
  
  // Quaternion data (for advanced orientation)
  float quatW;
  float quatX;
  float quatY;
  float quatZ;
  
  // Linear acceleration (gravity removed)
  float linearAccelX;
  float linearAccelY;
  float linearAccelZ;
  
  // Gravity vector
  float gravityX;
  float gravityY;
  float gravityZ;
};

/**
 * @brief Initializes the BNO055 sensor. Call this once in setup().
 * @return true if the sensor was found and initialized, false otherwise.
 */
bool setupBno055();

/**
 * @brief Reads all current data from the sensor and stores it internally.
 * Call this in every iteration of your main loop().
 */
void updateBno055Data();

/**
 * @brief Gets the most recently read sensor data.
 * @return A Bno055Data struct containing the latest information.
 */
Bno055Data getBno055Data();

/**
 * @brief Checks if the sensor is responding and providing valid data.
 * @return true if sensor is working properly.
 */
bool isBno055Ready();

#endif // NINE_AXIS_H