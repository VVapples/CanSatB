#include <Arduino.h>
#include "9axis.h"
#include "gps.h"
#include "sd_logger.h"
#include "ultrasonic.h"
#include "pose_est.h"
// #include "motor.h"  // Uncomment when motor system is ready


//pins: all in gpio pin numbers
#define BNO055_SDA_PIN 7 // D0
#define BNO055_SCL_PIN 8 // D1

#define GPS_RX_PIN 10 // D3
#define GPS_TX_PIN 9 // D2

#define ULTRASONIC_TRIGGER_PIN 1
#define ULTRASONIC_ECHO_PIN 3

#define SD_CD_PIN 5
#define SD_CMD_PIN 23
#define SD_CLK_PIN 18
#define SD_DATA0_PIN 19

#define Motor_AIN1_PIN nullptr //setlater
#define Motor_AIN2_PIN nullptr //setlater
#define Motor_BIN1_PIN nullptr //setlater
#define Motor_BIN2_PIN nullptr //setlater

#define LED_PIN nullptr //setlater


//important variables
static String state = "";
static String state_description = "";
static PositionData targetPos = {0.0, 0.0}; // Target position data

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

void setup() {
  state = "setup";
  state_description = "System is setting up";
  Serial.begin(115200);

  //SDcard setup : if failed with errors
  if (!setupSdLogger(SD_CD_PIN)) {
    state = "error";
    state_description = "SD Card initialization failed!";
    while (true) {
      // Stay here forever if SD card fails to initialize
      delay(1000);
    }
  } else {
    // setup logging
    writeLogHeaders("system.csv", "timestamp,state,code,message");
    writeToLog("system.csv", String(millis()) + ",SD,INIT_SUCCESS,0,SD card initialized successfully");
  }

  // Other sensor setups

  // BNO055
  if (!setupBno055()) {
    state = "error";
    state_description = "BNO055 initialization failed!";
    while (true) {
      // Stay here forever if BNO055 fails to initialize
      delay(1000);
    }
  } else {
    writeToLog("system.csv", String(millis()) + ",BNO055,INIT_SUCCESS,0,BNO055 initialized successfully");
  }

  // GPS
  if (!initializeGpsWithIndependentPower(GPS_TX_PIN, GPS_RX_PIN, 1, 9600)) {
    state = "error";
    state_description = "GPS initialization failed!";
    while (true) {
      // Stay here forever if GPS fails to initialize
      delay(1000);
    }
  } else {
    writeToLog("system.csv", String(millis()) + ",GPS,INIT_SUCCESS,0,GPS initialized successfully");
  }

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
  // Get target position from SD card
  targetPos = getTargetPosition();

  // Initialize pose logging
  setupPoseLogging();
  
  // Initialize motor system (uncomment when motor pins are set)
  // setupMotors(Motor_AIN1_PIN, Motor_AIN2_PIN, Motor_BIN1_PIN, Motor_BIN2_PIN);

  // Log system startup completion
  state_description = "All systems initialized successfully";
  writeToLog("system.csv", String(millis()) + ",SYSTEM,STARTUP_COMPLETE,0,All sensors initialized and system ready");
}

void loop() {
  state = "running";

  // Periodic logging
  static unsigned long lastLogTime = 0;
  if (millis() - lastLogTime >= 5000) {
    writeToLog("system.csv", String(millis()) + ",SYSTEM,LOOP_RUNNING,0,System main loop running - State: " + state + ", Description: " + state_description);
    lastLogTime = millis();
  }

  // Update Sensor data
  static bool sensorsUpdated = false;
  
  // Update BNO055 data
  static unsigned long lastBnoUpdate = 0;
  if (millis() - lastBnoUpdate >= 50) { // Update BNO055 every 50ms (20Hz)
    if (updateBno055Data()) {
      bnoData = getBno055Data(); // Update global variable
      sensorsUpdated = true;
    }
    lastBnoUpdate = millis();
  }

  // Update GPS data
  static unsigned long lastGpsUpdate = 0;
  if (millis() - lastGpsUpdate >= 1000) { // Update GPS every second
    if (updateGps()) {
      gpsData = getGpsData(); // Update global variable
      sensorsUpdated = true;
    }
    lastGpsUpdate = millis();
  }

  // === EKF POSE ESTIMATION ===
  // This is the key part - run EKF with current sensor data
  static bool ekfInitialized = false;
  
  if (!ekfInitialized) {
    // Initialize EKF when we have valid GPS fix
    if (gpsData.hasFix && gpsData.moduleDetected && 
        gpsData.latitude != 0.0 && gpsData.longitude != 0.0) {
      
      initializePoseEstimation(); // Initialize the EKF
      ekfInitialized = true;
      
      writeToLog("system.csv", String(millis()) + ",EKF,INIT_SUCCESS,0,EKF initialized with GPS fix");
      state_description = "EKF initialized - pose estimation active";
    } else {
      state_description = "Waiting for GPS fix to initialize EKF";
    }
  }
  
  // Run EKF pose estimation (this handles its own timing internally)
  if (ekfInitialized) {
    Pose currentPose = updatePoseEstimation(gpsData, bnoData);
    
    // Use the pose estimate for navigation decisions
    static unsigned long lastPoseAction = 0;
    if (millis() - lastPoseAction >= 2000) { // Every 2 seconds
      
      // Log current pose for monitoring
      String poseStatus = "Lat:" + String(currentPose.latitude, 6) + 
                         " Lon:" + String(currentPose.longitude, 6) +
                         " Hdg:" + String(currentPose.heading, 1) +
                         " Spd:" + String(currentPose.speed, 1) +
                         " Conf:" + String(currentPose.confidence, 2);
      
      writeToLog("system.csv", String(millis()) + ",POSE," + poseStatus);
      
      if (targetPos.latitude != 0.0 && targetPos.longitude != 0.0) {
        // Calculate direction and distance to target
        float targetDirection = estimateTargetDirection(currentPose.latitude, currentPose.longitude, 
                                                       targetPos.latitude, targetPos.longitude);
        float distanceToTarget = calculateDistanceToTarget(currentPose.latitude, currentPose.longitude,
                                                          targetPos.latitude, targetPos.longitude);
        
        // Log navigation info
        String navInfo = "Target Dir:" + String(targetDirection, 1) + 
                        " Dist:" + String(distanceToTarget, 1) + "m";
        writeToLog("system.csv", String(millis()) + ",NAV," + navInfo);
        
        // === MOTOR CONTROL BASED ON EKF ===
        if (distanceToTarget > 2.0) { // If more than 2 meters away
          
          // Calculate heading error
          float headingError = targetDirection - currentPose.heading;
          
          // Normalize heading error to -180 to +180
          while (headingError > 180.0) headingError -= 360.0;
          while (headingError < -180.0) headingError += 360.0;
          
          // Turn towards target if heading error is large
          if (abs(headingError) > 15.0) { // 15 degree tolerance
            // Uncomment when motors are connected:
            // motorTurn(headingError > 0 ? 10 : -10); // Turn 10 degrees towards target
            
            state_description = "Turning towards target (Error: " + String(headingError, 1) + "°)";
            writeToLog("system.csv", String(millis()) + ",MOTOR,TURN," + String(headingError, 1));
          }
          else {
            // Move forward towards target
            // Uncomment when motors are connected:
            // motorMove(1); // Move forward 1 unit
            
            state_description = "Moving towards target (" + String(distanceToTarget, 1) + "m away)";
            writeToLog("system.csv", String(millis()) + ",MOTOR,FORWARD," + String(distanceToTarget, 1));
          }
        }
        else {
          // Reached target!
          state_description = "Target reached! Distance: " + String(distanceToTarget, 1) + "m";
          writeToLog("system.csv", String(millis()) + ",NAV,TARGET_REACHED," + String(distanceToTarget, 1));
          
          // Stop motors
          // Uncomment when motors are connected:
          // motorStop();
        }
      }
      else {
        state_description = "No target position loaded";
      }
      
      lastPoseAction = millis();
    }
  }
  
  delay(50); // Reduced delay for more responsive system
}
