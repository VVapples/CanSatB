#include "9axis.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

// Create an instance of the BNO055 sensor
// The first parameter is an optional sensor ID, the second is the I2C address.
static Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

// A private (static) variable to hold our latest data
static BnoData currentBnoData;
// A flag to check if the sensor started successfully
static bool bnoInitialized = false;

bool setupBno() {
  // Attempt to initialize the BNO055. The mode specified, NDOF,
  // is "Nine Degrees of Freedom" fusion mode, which is the most powerful.
  if (!bno.begin(Adafruit_BNO055::OPERATION_MODE_NDOF)) {
    Serial.println("## Failed to find BNO055 sensor! Check wiring.");
    bnoInitialized = false;
    return false;
  }

  // Optional: Add a small delay to allow the sensor to stabilize
  delay(100);

  bnoInitialized = true;
  return true;
}

void updateBnoData() {
  // Don't try to read data if the sensor isn't initialized.
  if (!bnoInitialized) {
    return;
  }

  // 1. Get Fused Orientation Data (Euler Angles)
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  currentBnoData.heading = euler.x();
  currentBnoData.roll = euler.y();
  currentBnoData.pitch = euler.z();

  // 2. Get Fused Linear Acceleration (without gravity)
  imu::Vector<3> linearAccel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
  currentBnoData.linearAccelX = linearAccel.x();
  currentBnoData.linearAccelY = linearAccel.y();
  currentBnoData.linearAccelZ = linearAccel.z();

  // 3. Get Calibration Status
  bno.getCalibration(&currentBnoData.sys_cal, &currentBnoData.gyro_cal, &currentBnoData.accel_cal, &currentBnoData.mag_cal);
}

BnoData getBnoData() {
  return currentBnoData;
}

bool isBnoCalibrated() {
  if (!bnoInitialized) {
    return false;
  }
  // The sensor is fully calibrated only when all four values are 3.
  return (currentBnoData.sys_cal == 3 &&
          currentBnoData.gyro_cal == 3 &&
          currentBnoData.accel_cal == 3 &&
          currentBnoData.mag_cal == 3);
}