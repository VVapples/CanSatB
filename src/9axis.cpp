#include "9axis.h"
#include <Wire.h>
#include "MPU9250.h"
#include "sd_logger.h"

// Create an instance of the MPU9250 sensor with I2C parameters
// Using Wire (I2C) and default MPU9250 address (0x68)
static MPU9250 mpu(Wire, 0x68);

// A private (static) variable to hold our latest data
static Mpu9250Data currentMpu9250Data;
// A flag to check if the sensor started successfully
static bool mpu9250Initialized = false;

bool setupMpu9250() {
  // Initialize I2C communication
  Wire.begin();
  
  // Initialize log headers for MPU9250 data
  writeLogHeaders("mpu9250_data.csv", "Timestamp,AccelX,AccelY,AccelZ,GyroX,GyroY,GyroZ,MagX,MagY,MagZ,Pitch,Roll,Heading,Temperature");
  writeLogHeaders("system.csv", "Timestamp,Component,Event,Status,Details");
  
  // Start the MPU9250 sensor
  int status = mpu.begin();
  if (status < 0) {
    String errorMsg = "Failed to find MPU9250 sensor! Check wiring. Status: " + String(status);
    Serial.println("## " + errorMsg);
    
    // Log the error to SD card
    String logEntry = String(millis()) + ",MPU9250,INIT_FAILED," + String(status) + "," + errorMsg;
    writeToLog("system.csv", logEntry);
    
    mpu9250Initialized = false;
    return false;
  }

  // Configure the MPU9250 settings
  mpu.setAccelRange(MPU9250::ACCEL_RANGE_8G);     // ±8g accelerometer range
  mpu.setGyroRange(MPU9250::GYRO_RANGE_500DPS);   // ±500°/s gyroscope range
  mpu.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_20HZ); // Low-pass filter
  mpu.setSrd(19); // Sample rate divider (50 Hz update rate)

  // Optional: Add a small delay to allow the sensor to stabilize
  delay(100);

  String successMsg = "MPU9250 initialized successfully!";
  Serial.println(successMsg);
  
  // Log successful initialization
  String logEntry = String(millis()) + ",MPU9250,INIT_SUCCESS,0," + successMsg;
  writeToLog("system.csv", logEntry);
  
  mpu9250Initialized = true;
  return true;
}

void updateMpu9250Data() {
  // Don't try to read data if the sensor isn't initialized.
  if (!mpu9250Initialized) {
    return;
  }

  // Read sensor data
  mpu.readSensor();

  // 1. Get Accelerometer Data (in m/s^2)
  currentMpu9250Data.accelX = mpu.getAccelX_mss();
  currentMpu9250Data.accelY = mpu.getAccelY_mss();
  currentMpu9250Data.accelZ = mpu.getAccelZ_mss();

  // 2. Get Gyroscope Data (in degrees/second)
  currentMpu9250Data.gyroX = mpu.getGyroX_rads() * 180.0 / PI; // Convert to degrees
  currentMpu9250Data.gyroY = mpu.getGyroY_rads() * 180.0 / PI;
  currentMpu9250Data.gyroZ = mpu.getGyroZ_rads() * 180.0 / PI;

  // 3. Get Magnetometer Data (in microTesla)
  currentMpu9250Data.magX = mpu.getMagX_uT();
  currentMpu9250Data.magY = mpu.getMagY_uT();
  currentMpu9250Data.magZ = mpu.getMagZ_uT();

  // 4. Get Temperature (in Celsius)
  currentMpu9250Data.temperature = mpu.getTemperature_C();

  // 5. Calculate simple orientation estimates
  // Pitch and Roll from accelerometer (in degrees)
  currentMpu9250Data.pitch = atan2(-currentMpu9250Data.accelX, 
                                  sqrt(currentMpu9250Data.accelY * currentMpu9250Data.accelY + 
                                       currentMpu9250Data.accelZ * currentMpu9250Data.accelZ)) * 180.0 / PI;
  
  currentMpu9250Data.roll = atan2(currentMpu9250Data.accelY, currentMpu9250Data.accelZ) * 180.0 / PI;

  // Heading from magnetometer (simple 2D calculation)
  currentMpu9250Data.heading = atan2(currentMpu9250Data.magY, currentMpu9250Data.magX) * 180.0 / PI;
  if (currentMpu9250Data.heading < 0) {
    currentMpu9250Data.heading += 360.0; // Normalize to 0-360 degrees
  }
  
  // Log sensor data to SD card
  String dataEntry = String(millis()) + "," +
                    String(currentMpu9250Data.accelX, 3) + "," +
                    String(currentMpu9250Data.accelY, 3) + "," +
                    String(currentMpu9250Data.accelZ, 3) + "," +
                    String(currentMpu9250Data.gyroX, 2) + "," +
                    String(currentMpu9250Data.gyroY, 2) + "," +
                    String(currentMpu9250Data.gyroZ, 2) + "," +
                    String(currentMpu9250Data.magX, 1) + "," +
                    String(currentMpu9250Data.magY, 1) + "," +
                    String(currentMpu9250Data.magZ, 1) + "," +
                    String(currentMpu9250Data.pitch, 2) + "," +
                    String(currentMpu9250Data.roll, 2) + "," +
                    String(currentMpu9250Data.heading, 2) + "," +
                    String(currentMpu9250Data.temperature, 1);
  
  writeToLog("mpu9250_data.csv", dataEntry);
}

Mpu9250Data getMpu9250Data() {
  return currentMpu9250Data;
}

bool isMpu9250Ready() {
  return mpu9250Initialized;
}