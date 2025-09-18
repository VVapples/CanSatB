#include "9axis.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include "sd_logger.h"

// Create an instance of the BNO055 sensor
// Using default I2C address (0x28) and default delay (32ms)
static Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

// A private (static) variable to hold our latest data
static Bno055Data currentBno055Data;
// A flag to check if the sensor started successfully
static bool bno055Initialized = false;

bool setupBno055() {
  // Initialize I2C communication
  Wire.begin();
  
  // Initialize log headers for BNO055 data
  writeLogHeaders("bno055_data.csv", "Timestamp,AccelX,AccelY,AccelZ,GyroX,GyroY,GyroZ,MagX,MagY,MagZ,Pitch,Roll,Heading,Temperature,QuatW,QuatX,QuatY,QuatZ,LinearAccelX,LinearAccelY,LinearAccelZ,GravityX,GravityY,GravityZ");
  
  // Start the BNO055 sensor
  if (!bno.begin()) {
    String errorMsg = "Failed to find BNO055 sensor! Check wiring.";
    
    // Log the error to SD card
    String logEntry = String(millis()) + ",BNO055,INIT_FAILED,-1," + errorMsg;
    writeToLog("system.csv", logEntry);
    
    bno055Initialized = false;
    return false;
  }

  // Optional: Add a small delay to allow the sensor to stabilize
  delay(1000);
  
  // Set the external crystal use (this improves accuracy)
  bno.setExtCrystalUse(true);

  String successMsg = "BNO055 initialized successfully!";

  // Log successful initialization
  String logEntry = String(millis()) + ",BNO055,INIT_SUCCESS,0," + successMsg;
  writeToLog("system.csv", logEntry);
  
  bno055Initialized = true;
  return true;
}

bool updateBno055Data() {
  // Don't try to read data if the sensor isn't initialized.
  if (!bno055Initialized) {
    return false;
  }

  // Store previous data to detect changes
  Bno055Data previousData = currentBno055Data;

  // Get sensor events for different data types
  sensors_event_t accelEvent, gyroEvent, magEvent, linearAccelEvent, gravityEvent;
  
  // Read accelerometer data
  bno.getEvent(&accelEvent, Adafruit_BNO055::VECTOR_ACCELEROMETER);
  currentBno055Data.accelX = accelEvent.acceleration.x;
  currentBno055Data.accelY = accelEvent.acceleration.y;
  currentBno055Data.accelZ = accelEvent.acceleration.z;

  // Read gyroscope data
  bno.getEvent(&gyroEvent, Adafruit_BNO055::VECTOR_GYROSCOPE);
  currentBno055Data.gyroX = gyroEvent.gyro.x;
  currentBno055Data.gyroY = gyroEvent.gyro.y;
  currentBno055Data.gyroZ = gyroEvent.gyro.z;

  // Read magnetometer data
  bno.getEvent(&magEvent, Adafruit_BNO055::VECTOR_MAGNETOMETER);
  currentBno055Data.magX = magEvent.magnetic.x;
  currentBno055Data.magY = magEvent.magnetic.y;
  currentBno055Data.magZ = magEvent.magnetic.z;

  // Read linear acceleration (gravity removed)
  bno.getEvent(&linearAccelEvent, Adafruit_BNO055::VECTOR_LINEARACCEL);
  currentBno055Data.linearAccelX = linearAccelEvent.acceleration.x;
  currentBno055Data.linearAccelY = linearAccelEvent.acceleration.y;
  currentBno055Data.linearAccelZ = linearAccelEvent.acceleration.z;

  // Read gravity vector
  bno.getEvent(&gravityEvent, Adafruit_BNO055::VECTOR_GRAVITY);
  currentBno055Data.gravityX = gravityEvent.acceleration.x;
  currentBno055Data.gravityY = gravityEvent.acceleration.y;
  currentBno055Data.gravityZ = gravityEvent.acceleration.z;

  // Get orientation (Euler angles) - BNO055's fusion algorithm
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  currentBno055Data.heading = euler.x(); // Yaw (0-360 degrees)
  currentBno055Data.pitch = euler.y();   // Pitch (-180 to +180 degrees)  
  currentBno055Data.roll = euler.z();    // Roll (-90 to +90 degrees)

  // Get quaternion data
  imu::Quaternion quat = bno.getQuat();
  currentBno055Data.quatW = quat.w();
  currentBno055Data.quatX = quat.x();
  currentBno055Data.quatY = quat.y();
  currentBno055Data.quatZ = quat.z();

  // Get temperature
  currentBno055Data.temperature = bno.getTemp();
  
  // Check if we have new/valid data by comparing with previous data
  // BNO055 should always provide some data, but we check for sensor readiness
  bool hasNewData = true; // Assume new data unless sensor fails
  
  // Basic sanity check - if all readings are exactly 0, sensor might not be responding
  if (currentBno055Data.accelX == 0.0 && currentBno055Data.accelY == 0.0 && currentBno055Data.accelZ == 0.0 &&
      currentBno055Data.gyroX == 0.0 && currentBno055Data.gyroY == 0.0 && currentBno055Data.gyroZ == 0.0 &&
      currentBno055Data.magX == 0.0 && currentBno055Data.magY == 0.0 && currentBno055Data.magZ == 0.0) {
    hasNewData = false;
  }
  
  // Additional check: if data is exactly the same as previous reading and seems unrealistic
  if (hasNewData && 
      previousData.accelX == currentBno055Data.accelX && 
      previousData.accelY == currentBno055Data.accelY && 
      previousData.accelZ == currentBno055Data.accelZ &&
      previousData.gyroX == currentBno055Data.gyroX && 
      previousData.gyroY == currentBno055Data.gyroY && 
      previousData.gyroZ == currentBno055Data.gyroZ) {
    static int sameDataCount = 0;
    sameDataCount++;
    // If we get identical data more than 10 times, sensor might be stuck
    if (sameDataCount > 10) {
      hasNewData = false;
      sameDataCount = 0; // Reset counter
    }
  }
  
  // Only log data if we have valid new data
  if (hasNewData) {
    // Log sensor data to SD card
    String dataEntry = String(millis()) + "," +
                      String(currentBno055Data.accelX, 3) + "," +
                      String(currentBno055Data.accelY, 3) + "," +
                      String(currentBno055Data.accelZ, 3) + "," +
                      String(currentBno055Data.gyroX, 2) + "," +
                      String(currentBno055Data.gyroY, 2) + "," +
                      String(currentBno055Data.gyroZ, 2) + "," +
                      String(currentBno055Data.magX, 1) + "," +
                      String(currentBno055Data.magY, 1) + "," +
                      String(currentBno055Data.magZ, 1) + "," +
                      String(currentBno055Data.pitch, 2) + "," +
                      String(currentBno055Data.roll, 2) + "," +
                      String(currentBno055Data.heading, 2) + "," +
                      String(currentBno055Data.temperature, 1) + "," +
                      String(currentBno055Data.quatW, 4) + "," +
                      String(currentBno055Data.quatX, 4) + "," +
                      String(currentBno055Data.quatY, 4) + "," +
                      String(currentBno055Data.quatZ, 4) + "," +
                      String(currentBno055Data.linearAccelX, 3) + "," +
                      String(currentBno055Data.linearAccelY, 3) + "," +
                      String(currentBno055Data.linearAccelZ, 3) + "," +
                      String(currentBno055Data.gravityX, 3) + "," +
                      String(currentBno055Data.gravityY, 3) + "," +
                      String(currentBno055Data.gravityZ, 3);
    
    writeToLog("bno055_data.csv", dataEntry);
  }
  
  return hasNewData;
}

Bno055Data getBno055Data() {
  return currentBno055Data;
}

bool isBno055Ready() {
  return bno055Initialized;
}