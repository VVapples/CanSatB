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

//pins: all in gpio pin numbers
#define BNO055_SDA_PIN 7 // D0
#define BNO055_SCL_PIN 8 // D1

#define GPS_RX_PIN 10 // D3
#define GPS_TX_PIN 9 // D2

#define ULTRASONIC_TRIGGER_PIN   1
#define ULTRASONIC_ECHO_PIN      3

#define SD_CD_PIN      5
#define SD_CMD_PIN     23
#define SD_CLK_PIN     18
#define SD_DATA0_PIN   19

#define MOTOR_STBY    15    // Standby pin - LOW = standby, HIGH = active
#define MOTOR_A_PWM   27    // Motor A PWM (speed control)
#define MOTOR_A_IN1   12    // Motor A direction pin 1
#define MOTOR_A_IN2   14    // Motor A direction pin 2
#define MOTOR_B_PWM   33    // Motor B PWM (speed control)
#define MOTOR_B_IN1   2     // Motor B direction pin 1
#define MOTOR_B_IN2   4     // Motor B direction pin 2

#define LED_PIN nullptr //setlater

//operation related constants
static const float CLOSEIN_START_THRESHOLD = 5.0; // in meters
static const float TARGET_REACHED_THRESHOLD = 2.0; // in meters

//important variables
static String state = "";
static String state_description = "";
static double distanceToTarget = 0.0;
static double headingError = 0.0;
static unsigned int turnCycle = 0;
static unsigned int turnCycleTimes = 0;

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
  state = "setup";
  state_description = "System is setting up";

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

  // Ultrasonic
  if (!setupUltrasonic(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN)) {
    state = "error";
    state_description = "Ultrasonic sensor initialization failed!";
    writeToLog("system.csv", String(millis()) + ",ULTRASONIC,INIT_FAILED,-1,Ultrasonic sensor initialization failed!");
    while (true) {
      // Stay here forever if Ultrasonic sensor fails to initialize
      delay(1000);
    }
  } else {
    writeToLog("system.csv", String(millis()) + ",ULTRASONIC,INIT_SUCCESS,0,Ultrasonic sensor initialized successfully");
  }

  // Initialize motors with pin assignments from main.cpp
  setupMotors(MOTOR_STBY, MOTOR_A_PWM, MOTOR_A_IN1, MOTOR_A_IN2, MOTOR_B_PWM, MOTOR_B_IN1, MOTOR_B_IN2);
  writeToLog("system.csv", String(millis()) + ",MOTOR,INIT_SUCCESS,0,Motors initialized successfully");
  writeLogHeaders("control.csv", "timestamp,action_description");

  // Initialize pose logging
  setupPoseLogging();

  // Get target coordinates from SD card
  File targetFile = SD.open("/targetCoordinate.csv");
  if (targetFile) {
    String line = "";
    bool headerSkipped = false;
    
    while (targetFile.available()) {
      line = targetFile.readStringUntil('\n');
      line.trim();
      
      // Skip header line if it exists
      if (!headerSkipped && (line.startsWith("lat") || line.startsWith("Lat") || line.startsWith("LAT"))) {
        headerSkipped = true;
        continue;
      }
      
      // Parse CSV line: latitude,longitude
      int commaIndex = line.indexOf(',');
      if (commaIndex > 0) {
        String latStr = line.substring(0, commaIndex);
        String lonStr = line.substring(commaIndex + 1);
        
        targetCoordinates.latitude = latStr.toDouble();
        targetCoordinates.longitude = lonStr.toDouble();
        targetCoordinates.heading = 0.0; // Not used for target
        
        writeToLog("system.csv", String(millis()) + ",TARGET,COORDINATES_LOADED,0,Target coordinates set to " + 
                   String(targetCoordinates.latitude, 6) + "," + String(targetCoordinates.longitude, 6));
        break; // Use first valid line
      }
    }
    targetFile.close();
    
    // Validate coordinates are reasonable
    if (targetCoordinates.latitude == 0.0 && targetCoordinates.longitude == 0.0) {
      writeToLog("system.csv", String(millis()) + ",TARGET,COORDINATES_INVALID,-1,Invalid target coordinates (0,0) - check targetCoordiante.csv format");
    }
  } else {
    writeToLog("system.csv", String(millis()) + ",TARGET,FILE_NOT_FOUND,-1,targetCoordiante.csv file not found on SD card");
    // Set default coordinates if file not found
    targetCoordinates.latitude = 0.0;
    targetCoordinates.longitude = 0.0;
    targetCoordinates.heading = 0.0;
  }
  
  // Log system startup completion
  state_description = "All systems initialized successfully";
  writeToLog("system.csv", String(millis()) + ",SYSTEM,STARTUP_COMPLETE,0,All sensors initialized and system ready");

}

void loop() {

  // Periodic logging
  static unsigned long lastLogTime = 0;
  if (millis() - lastLogTime >= 5000) {
    writeToLog("system.csv", String(millis()) + ",SYSTEM,LOOP_RUNNING,0,System main loop running - State: " + state + ", Description: " + state_description);
    lastLogTime = millis();
  }

  // Update Sensor data
  // Update BNO055 data
  static unsigned long lastBnoUpdate = 0;
  if (millis() - lastBnoUpdate >= 100) { // Update BNO055 every 100 ms
    static bool bnoUpdated = false;
    bnoUpdated = updateBno055Data();
    lastBnoUpdate = millis();
    if (bnoUpdated) {
      Bno055Data bnoData = getBno055Data();
    }
  }

  // Update GPS data
  static unsigned long lastGpsUpdate = 0;
  if (millis() - lastGpsUpdate >= 1000) { // Update GPS every second
    static bool updated = false;
    updated = updateGps();
    lastGpsUpdate = millis();
    if (updated) {
      GpsData gpsData = getGpsData();
    }
  }

  // get ultrasonic distance
  if (state == "CloseIn") {
    static unsigned long lastUltrasonicUpdate = 0;
    if (millis() - lastUltrasonicUpdate >= 200) { // Update Ultrasonic every 200 ms
      float distance = getDistanceCm();
      lastUltrasonicUpdate = millis();
    }
  }

  // Get current pose estimate
  static unsigned long lastPoseLog = 0;
  if (lastGpsUpdate > lastPoseLog || lastBnoUpdate > lastPoseLog) {
    // Get current sensor data and calculate pose
    GpsData currentGpsData = getGpsData();
    Bno055Data currentBnoData = getBno055Data();
    
    Pose currentPose = getCurrentPose(currentGpsData, currentBnoData);
    lastPoseLog = millis();
  }

  //state decider
  static unsigned long lastStateChangeTime = 0;
  distanceToTarget = calculateDistance(currentPose.latitude, currentPose.longitude, targetCoordinates.latitude, targetCoordinates.longitude);
  if (millis() - lastStateChangeTime > 10000) {  // Evaluate state every 10 seconds
    lastStateChangeTime = millis();
    if (state == "setup" && distanceToTarget > CLOSEIN_START_THRESHOLD) {
      state = "approach";
      state_description = "Searching for target - distance to target: " + String(distanceToTarget, 2) + " meters";
      writeToLog("system.csv", String(millis()) + ",STATE,APPROACH,0,Transitioning to APPROACH state - distance to target: " + String(distanceToTarget, 2) + " meters");
    } else if (state == "approach" && distanceToTarget > TARGET_REACHED_THRESHOLD) {
      state = "closeIn";
      state_description = "Approaching target - distance to target: " + String(distanceToTarget, 2) + " meters";
      writeToLog("system.csv", String(millis()) + ",STATE,CLOSEIN,0,Transitioning to CLOSEIN state - distance to target: " + String(distanceToTarget, 2) + " meters");
    } else {
      state = "arrived";
      state_description = "Arrived at target location - distance to target: " + String(distanceToTarget, 2) + " meters";
      writeToLog("system.csv", String(millis()) + ",STATE,ARRIVED,0,Transitioning to ARRIVED state - distance to target: " + String(distanceToTarget, 2) + " meters");
      stopAllMotors();
      writeToLog("control.csv", String(millis()) + ",Motors stopped - Arrived at target");
      while (true) {
        // Stay here forever if arrived
        delay(1000);
      }
    }
  } 

  // Motor control and other operations would go here

  //some random motor control for testing
  if (state == "approach") {
    headingError = calculateHeadingError(currentPose, targetCoordinates);
    if (abs(headingError) > 30) {
      if (headingError > 0) {
        //turn right
        motorWrite('A', -100);
        motorWrite('B', 100);
        headingError = calculateHeadingError(currentPose, targetCoordinates);
        writeToLog("control.csv", String(millis()) + ",Turning right to correct heading - Error: " + String(headingError, 2) + " degrees");
      } else {
        // Turn left
        motorWrite('A', 100);
        motorWrite('B', -100);
        headingError = calculateHeadingError(currentPose, targetCoordinates);
        writeToLog("control.csv", String(millis()) + ",Turning left to correct heading - Error: " + String(headingError, 2) + " degrees");
      }
    } else {
      // Move forward with some direction correction
      motorWrite('A', 150 + int(headingError));
      motorWrite('B', 150 - int(headingError));
      writeToLog("control.csv", String(millis()) + ",Moving forward towards target - Heading error: " + String(headingError, 2) + " degrees");
    }
  } else if (state == "closeIn") {
    float distance = getDistanceCm();
    if (distance <= 0 || distance > 400) { // Assuming 400 cm is the max range
      // cycle limiter
      if (turnCycleTimes >= 3) {
        // After several cycles, stop and log error
        motorWrite('A', 0);
        motorWrite('B', 0);
        writeToLog("control.csv", String(millis()) + ",Ultrasonic sensor failure - motors stopped");
        state = "error";
        state_description = "Ultrasonic sensor failure - motors stopped";
        writeToLog("system.csv", String(millis()) + ",ULTRASONIC,FAILURE,-1,Ultrasonic sensor failed to detect target after multiple attempts - motors stopped");
        while (true) {
          delay(1000); // Stay here forever
        }
      } else {
        // Timeout or invalid reading - try turning to find target
        writeToLog("control.csv", String(millis()) + ",Ultrasonic timeout or invalid reading - attempting to reorient");
      }
      // adjusting direction untill ultrasonic detects something
      if (turnCycle <= 2) {
        // turn right
        motorWrite('A', -100);
        motorWrite('B', 100);
        writeToLog("control.csv", String(millis()) + ",Turning right to find target - Ultrasonic distance invalid");
        turnCycle++;
      } else if (turnCycle >= 3 && turnCycle <= 5) {
        // turn left
        motorWrite('A', 100);
        motorWrite('B', -100);
        writeToLog("control.csv", String(millis()) + ",Turning left to find target - Ultrasonic distance invalid");
        turnCycle++;
      } else {
        turnCycle = 0; // reset cycle
        turnCycleTimes++;
      } 
    } else {
      // Valid distance reading - move forward
      motorWrite('A', 100);
      motorWrite('B', 100);
      writeToLog("control.csv", String(millis()) + ",Moving forward - Ultrasonic distance: " + String(distance, 2) + " cm");
      // Reset cycle counters
      turnCycle = 0;
      turnCycleTimes = 0;
    }
  } else if (state == "arrived") {
    // fallback for when state check fails to stop program
    stopAllMotors();
    writeToLog("control.csv", String(millis()) + ",Motors stopped - Arrived at target ### LOGIC ERROR");
    delay(1000); // Just to avoid spamming the log (wait for state change logic to stop program)
  }

  
  delay(100); // debounce
}
