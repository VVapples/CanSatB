#include <Arduino.h>
#include "SD.h"
#include "FS.h"
#include "9axis.h"
#include "gps.h"
#include "sd_logger.h"
#include "ultrasonic.h"
#include "pose_est.h"
#include "motor.h"
#include "calculations.h"
#include "task0.h"
#include "task1.h"
#include "CONSTANTS.h"

// Initialize BNO055 data structure with default values
Bno055Data bnoData = {
  0.0, 0.0, 0.0,        // accelX, accelY, accelZ
  0.0, 0.0, 0.0,        // gyroX, gyroY, gyroZ  
  0.0, 0.0, 0.0,        // magX, magY, magZ
  0.0,                  // temperature
  0.0, 0.0, 0.0,        // pitch, roll, heading
  1.0, 0.0, 0.0, 0.0,   // quatW, quatX, quatY, quatZ (identity quaternion)
  0.0, 0.0, 0.0,        // linearAccelX, linearAccelY, linearAccelZ
  0.0, 0.0, 0.0         // gravityX, gravityY, gravityZ
};

// Initialize GPS data structure with default values
GpsData gpsData = {
  0.0, 0.0, 0.0,        // latitude, longitude, altitude
  false, false, 0,      // hasFix, moduleDetected, satelliteCount
  99.99, 0, 1,          // hdop (high = poor), fixQuality, fixType
  0.0, 0.0,             // speed, course
  false, 0, 0,          // timeValid, date, time
  0                     // lastUpdate
};

// Initialize current pose with default values
Pose currentPose = {
  0.0,       // latitude
  0.0,       // longitude
  0.0        // heading
};

// Initialize target coordinates structure
Pose targetCoordinates = {
  0.0,       // latitude
  0.0,       // longitude
  0.0        // heading (not used for target)
};

void setup() {

  

  xTaskCreatePinnedToCore(
    task0Loop,           // Task function
    "LogWriter",         // Task name
    4096,                // Stack size (bytes)
    NULL,                // Parameters
    1,                   // Priority (0-25, higher = more priority)
    &task0Handle,        // Task handle
    0                    // Core number (0)
  );

  xTaskCreatePinnedToCore(
    task0Loop,           // Task function
    "LogWriter",         // Task name
    4096,                // Stack size (bytes)
    NULL,                // Parameters
    1,                   // Priority (0-25, higher = more priority)
    &task1Handle,        // Task handle
    0                    // Core number (0)
  );

    xTaskCreatePinnedToCore(
    task0Loop,           // Task function
    "LogWriter",         // Task name
    4096,                // Stack size (bytes)
    NULL,                // Parameters
    1,                   // Priority (0-25, higher = more priority)
    &task2Handle,        // Task handle
    0                    // Core number (0)
  );
  // state = "setup";
  // state_description = "System is setting up";

  // //SDcard setup : if failed with errors
  // if (!setupSdLogger(SD_CD_PIN)) {
  //   state = "error";
  //   state_description = "SD Card initialization failed!";
  //   while (true) {
  //     // Stay here forever if SD card fails to initialize
  //     delay(1000);
  //   }
  // } else {
  //   // setup logging
  //   writeLogHeaders("system.csv", "timestamp,state,code,message");
  //   writeToLog("system.csv", String(millis()) + ",SD,INIT_SUCCESS,0,SD card initialized successfully");
  // }

  // Other sensor setups

  // BNO055
  // if (!setupBno055()) {
  //   state = "error";
  //   state_description = "BNO055 initialization failed!";
  //   while (true) {
  //     // Stay here forever if BNO055 fails to initialize
  //     delay(1000);
  //   }
  // } else {
  //   writeToLog("system.csv", String(millis()) + ",BNO055,INIT_SUCCESS,0,BNO055 initialized successfully");
  // }

  // // GPS
  // if (!setupGps(GPS_TX_PIN, GPS_RX_PIN)) {
  //   state = "error";
  //   state_description = "GPS initialization failed!";
  //   while (true) {
  //     // Stay here forever if GPS fails to initialize
  //     delay(1000);
  //   }
  // } else {
  //   writeToLog("system.csv", String(millis()) + ",GPS,INIT_SUCCESS,0,GPS initialized successfully");
  // }

  // // Ultrasonic
  // if (!setupUltrasonic(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN)) {
  //   state = "error";
  //   state_description = "Ultrasonic sensor initialization failed!";
  //   writeToLog("system.csv", String(millis()) + ",ULTRASONIC,INIT_FAILED,-1,Ultrasonic sensor initialization failed!");
  //   while (true) {
  //     // Stay here forever if Ultrasonic sensor fails to initialize
  //     delay(1000);
  //   }
  // } else {
  //   writeToLog("system.csv", String(millis()) + ",ULTRASONIC,INIT_SUCCESS,0,Ultrasonic sensor initialized successfully");
  // }

  // // Initialize motors with pin assignments from main.cpp
  // setupMotors(MOTOR_STBY, MOTOR_A_PWM, MOTOR_A_IN1, MOTOR_A_IN2, MOTOR_B_PWM, MOTOR_B_IN1, MOTOR_B_IN2);
  // writeToLog("system.csv", String(millis()) + ",MOTOR,INIT_SUCCESS,0,Motors initialized successfully");

  // // Initialize pose logging
  // setupPoseLogging();

  // // Get target coordinates from SD card
  // File targetFile = SD.open("/targetCoordinate.csv");
  // if (targetFile) {
  //   String line = "";
  //   bool headerSkipped = false;
    
  //   while (targetFile.available()) {
  //     line = targetFile.readStringUntil('\n');
  //     line.trim();
      
  //     // Skip header line if it exists
  //     if (!headerSkipped && (line.startsWith("lat") || line.startsWith("Lat") || line.startsWith("LAT"))) {
  //       headerSkipped = true;
  //       continue;
  //     }
      
  //     // Parse CSV line: latitude,longitude
  //     int commaIndex = line.indexOf(',');
  //     if (commaIndex > 0) {
  //       String latStr = line.substring(0, commaIndex);
  //       String lonStr = line.substring(commaIndex + 1);
        
  //       targetCoordinates.latitude = latStr.toDouble();
  //       targetCoordinates.longitude = lonStr.toDouble();
  //       targetCoordinates.heading = 0.0; // Not used for target
        
  //       writeToLog("system.csv", String(millis()) + ",TARGET,COORDINATES_LOADED,0,Target coordinates set to " + 
  //                  String(targetCoordinates.latitude, 6) + "," + String(targetCoordinates.longitude, 6));
  //       break; // Use first valid line
  //     }
  //   }
  //   targetFile.close();
    
  //   // Validate coordinates are reasonable
  //   if (targetCoordinates.latitude == 0.0 && targetCoordinates.longitude == 0.0) {
  //     writeToLog("system.csv", String(millis()) + ",TARGET,COORDINATES_INVALID,-1,Invalid target coordinates (0,0) - check targetCoordiante.csv format");
  //   }
  // } else {
  //   writeToLog("system.csv", String(millis()) + ",TARGET,FILE_NOT_FOUND,-1,targetCoordiante.csv file not found on SD card / Creating default file");
  //   // Set default coordinates if file not found
  //   writeLogHeaders("targetCoordinate.csv", "lat,lon");
  //   writeToLog("targetCoordinate.csv", "0.0,0.0");
  //   targetCoordinates.latitude = 0.0;
  //   targetCoordinates.longitude = 0.0;
  //   targetCoordinates.heading = 0.0;
  // }
  
  // Log system startup completion
  // state_description = "All systems initialized successfully";
  // writeToLog("system.csv", String(millis()) + ",SYSTEM,STARTUP_COMPLETE,0,All sensors initialized and system ready");

}

void loop() {

  //KEEP EMPTY

  // Periodic logging
  // static unsigned long lastLogTime = 0;
  // if (millis() - lastLogTime >= 5000) {
  //   writeToLog("system.csv", String(millis()) + ",SYSTEM,LOOP_RUNNING,0,System main loop running - State: " + state + " | Description: " + state_description);
  //   lastLogTime = millis();
  // }

  // Update Sensor data
  // Update BNO055 data
  // static unsigned long lastBnoUpdate = 0;
  // if (millis() - lastBnoUpdate >= 100) { // Update BNO055 every 100 ms
  //   static bool bnoUpdated = false;
  //   bnoUpdated = updateBno055Data();
  //   lastBnoUpdate = millis();
  //   if (bnoUpdated) {
  //     Bno055Data bnoData = getBno055Data();
  //   }
  // }

  // // Update GPS data
  // static unsigned long lastGpsUpdate = 0;
  // if (millis() - lastGpsUpdate >= 1000) { // Update GPS every second
  //   static bool updated = false;
  //   updated = updateGps();
  //   lastGpsUpdate = millis();
  //   if (updated) {
  //     GpsData gpsData = getGpsData();
  //   }
  // }

  // // get ultrasonic distance
  // if (state == "CloseIn") {
  //   static unsigned long lastUltrasonicUpdate = 0;
  //   if (millis() - lastUltrasonicUpdate >= 200) { // Update Ultrasonic every 200 ms
  //     float distance = getDistanceCm();
  //     lastUltrasonicUpdate = millis();
  //   }
  // }

  // // Get current pose estimate
  // static unsigned long lastPoseLog = 0;
  // if (lastGpsUpdate > lastPoseLog || lastBnoUpdate > lastPoseLog) {
  //   // Get current sensor data and calculate pose
  //   GpsData currentGpsData = getGpsData();
  //   Bno055Data currentBnoData = getBno055Data();
    
  //   Pose currentPose = getCurrentPose(currentGpsData, currentBnoData);
  //   lastPoseLog = millis();
  // }

  // //state decider
  // static unsigned long lastStateChangeTime = 0;
  // if (millis() - lastStateChangeTime > 10000) {  // Evaluate state every 10 seconds
  //   lastStateChangeTime = millis();
  //   static double distanceToTarget = calculateDistance(currentPose.latitude, currentPose.longitude, targetCoordinates.latitude, targetCoordinates.longitude);
  //   if (state == "startup!" && distanceToTarget > CLOSEIN_START_THRESHOLD) {
  //     state = "approach";
  //     state_description = "Searching for target - distance to target: " + String(distanceToTarget, 2) + " meters";
  //     writeToLog("system.csv", String(millis()) + ",STATE,APPROACH,0,Transitioning to APPROACH state - distance to target: " + String(distanceToTarget, 2) + " meters");
  //   } else if (state == "approach" && distanceToTarget > TARGET_REACHED_THRESHOLD) {
  //     state = "closeIn";
  //     state_description = "Approaching target - distance to target: " + String(distanceToTarget, 2) + " meters";
  //   } else {
  //     state = "arrived";
  //     state_description = "Arrived at target location - distance to target: " + String(distanceToTarget, 2) + " meters";
  //   }
  // } 

  // // Motor control and other operations would go here

  // //some random motor control for testing
  // static unsigned long motorTestCycle = 0;
  // static unsigned long lastMotorTestTime = 0;
  // if (millis() - lastMotorTestTime > 5000) { // every 5 seconds
  //   lastMotorTestTime = millis();
  //   if (motorTestCycle == 0) {
  //     motorTestCycle = 1;
  //     exitStandby();
  //     motorWrite('A', 100); // Move forward at half speed
  //   } else if (motorTestCycle == 1) {
  //     motorTestCycle = 2;
  //     motorWrite('A', -100); // Move backward at half speed
  //     motorWrite('B', 100);  // Turn in place
  //   } else if (motorTestCycle == 2) {
  //     motorTestCycle = 3;
  //     motorWrite('A', 100); // Turn in place
  //     motorWrite('B', 100);
  //   } else if (motorTestCycle == 3) {
  //     motorTestCycle = 0;
  //     enterStandby(); // Stop motors
  //   }
  //   motorTestCycle++;
  // }
  

  delay(1000); // debounce
}
