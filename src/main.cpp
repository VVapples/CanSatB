//FOR TESTING PURPOSES 
//full logging and some motor control logic added

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
#include "led.h"

//pins: all in gpio pin numbers
#define BNO055_SDA_PIN 21 // D0
#define BNO055_SCL_PIN 22 // D1

#define GPS_RX_PIN 17
#define GPS_TX_PIN 16

#define ULTRASONIC_TRIGGER_PIN   26
#define ULTRASONIC_ECHO_PIN      25

#define SD_CD_PIN      13
#define SD_CMD_PIN     23
#define SD_CLK_PIN     18
#define SD_DATA0_PIN   19

#define MOTOR_STBY    3X3    // Standby pin - LOW = standby, HIGH = active
#define MOTOR_A_PWM   3X3    // Motor A PWM (speed control)
#define MOTOR_A_IN1   27    // Motor A direction pin 1
#define MOTOR_A_IN2   14    // Motor A direction pin 2
#define MOTOR_B_PWM   3X3    // Motor B PWM (speed control)
#define MOTOR_B_IN1   2     // Motor B direction pin 1
#define MOTOR_B_IN2   4     // Motor B direction pin 2

#define LED_PIN 32 //setlater

//operation related constants
static const float CLOSEIN_START_THRESHOLD = 5.0; // in meters
static const float TARGET_REACHED_THRESHOLD = 2.0; // in meters
static const float ULTRASONIC_ERROR_THRESHOLD = 5.0; // in cm, distances below this are considered errors

//important variables
static String state = "";
static String state_description = "";

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

// // Initialize target coordinates structure
Pose targetCoordinates = {
  0.0,       // latitude
  0.0,       // longitude
  0.0        // heading (not used for target)
};

// initialize distance variable
float distance = -1.0; // in cm, -1 indicates uninitialized or error

void setup() {
  state = "setup";
  state_description = "System is setting up";
  delay(1000); // Allow time for serial monitor to start
  Serial.begin(115200);
  setupLed(LED_PIN);
  ledMessage("startup");

  //SDcard setup : if failed with errors
  if (!setupSdLogger(SD_CD_PIN)) {
    state = "error";
    state_description = "SD Card initialization failed!";
    while (true) {
      // Stay here forever if SD card fails to initialize
      delay(1000);
      ledMessage("error");
    }
  } else {
    // setup logging
    writeLogHeaders("system.csv", "timestamp,state,code,message");
    writeToLog("system.csv", String(millis()) + ",SD,INIT_SUCCESS,0,SD card initialized successfully");
  }

  // Other sensor setups
  delay(1000); // Short delay to ensure SD card is ready
  // BNO055
  if (!setupBno055()) {
    state = "error";
    state_description = "BNO055 initialization failed!";
    writeToLog("system.csv", String(millis()) + ",BNO055,INIT_FAILED,-1,BNO055 initialization failed!");
    while (true) {
      // Stay here forever if BNO055 fails to initialize
      ledMessage("error");
      delay(1000);
    }
  } else {
    writeToLog("system.csv", String(millis()) + ",BNO055,INIT_SUCCESS,0,BNO055 initialized successfully");
  }

  // GPS
  if (!setupGps(GPS_TX_PIN, GPS_RX_PIN)) {
    state = "error";
    state_description = "GPS initialization failed!";
    writeToLog("system.csv", String(millis()) + ",GPS,INIT_FAILED,-1,GPS initialization failed!");
    while (true) {
      // Stay here forever if GPS fails to initialize
      ledMessage("error");
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
      ledMessage("error");
      delay(1000);
    }
  } else {
    writeToLog("system.csv", String(millis()) + ",ULTRASONIC,INIT_SUCCESS,0,Ultrasonic sensor initialized successfully");
  }

  // Initialize motors with pin assignments from main.cpp
  setupMotors(MOTOR_A_IN1, MOTOR_A_IN2, MOTOR_B_IN1, MOTOR_B_IN2);
  writeToLog("system.csv", String(millis()) + ",MOTOR,INIT_SUCCESS,0,Motors initialized successfully");

  // Initialize pose logging
  setupPoseLogging();

  // Get target coordinates from SD card
  delay(1000); //wait till write ends
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
    writeToLog("system.csv", String(millis()) + ",TARGET,FILE_NOT_FOUND,-1,targetCoordiante.csv file not found on SD card / Creating default file");
    // Set default coordinates if file not found
    writeLogHeaders("targetCoordinate.csv", "lat,lon");
    writeToLog("targetCoordinate.csv", "0.0,0.0");
    targetCoordinates.latitude = 0.0;
    targetCoordinates.longitude = 0.0;
    targetCoordinates.heading = 0.0;
  }
  
  // Log system startup completion
  state_description = "All systems initialized successfully";
  writeToLog("system.csv", String(millis()) + ",SYSTEM,STARTUP_COMPLETE,0,All sensors initialized and system ready");

}

void loop() {
  ledMessage("running");
  // Periodic logging
  static unsigned long lastLogTime = 0;
  if (millis() - lastLogTime >= 5000) {
    writeToLog("system.csv", String(millis()) + ",SYSTEM,LOOP_RUNNING,0,System main loop running - State: " + state + " | Description: " + state_description);
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
    static unsigned long lastUltrasonicUpdate = 0;
    if (millis() - lastUltrasonicUpdate >= 200) { // Update Ultrasonic every 200 ms
      distance = getDistanceCm();
      lastUltrasonicUpdate = millis();
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
  if (millis() - lastStateChangeTime > 10000) {  // Evaluate state every 10 seconds
    lastStateChangeTime = millis();
    static double distanceToTarget = calculateDistance(currentPose.latitude, currentPose.longitude, targetCoordinates.latitude, targetCoordinates.longitude);
    if (state == "setup") {
      state = "approach";
      state_description = "Searching for target - distance to target: " + String(distanceToTarget, 2) + " meters";
      writeToLog("system.csv", String(millis()) + ",STATE,APPROACH,0,Transitioning to APPROACH state - distance to target: " + String(distanceToTarget, 2) + " meters");
    } else if (state == "approach" && distanceToTarget <= CLOSEIN_START_THRESHOLD) {
      state = "closeIn";
      state_description = "Approaching target - distance to target: " + String(distanceToTarget, 2) + " meters";
    } else if (state == "closeIn" && ULTRASONIC_ERROR_THRESHOLD < distance && distance <= TARGET_REACHED_THRESHOLD) {
      state = "arrived";
      state_description = "Arrived at target location - distance to target: " + String(distanceToTarget, 2) + " meters";
    }
  } 

  // // Motor control and other operations would go here

  static double currentBearing = calculateBearing(currentPose.latitude, currentPose.longitude, targetCoordinates.latitude, targetCoordinates.longitude);
  if (state == "approach") {
    if (abs(currentBearing) > 30) {
      if (currentBearing > 0) {
        motorControl("right", 100); // Turn right
      } else {
        // Turn left
        motorControl("left", 100); // Turn left
      }
    } else {
      // Move forward
      motorControl("forward", 150); // Move forward
    }
  } else if (state == "closeIn") {
        static int searchTimes = 0;
        static bool found = false;
        while (searchTimes < 10 && !found) {
          distance = getDistanceCm();
          lastUltrasonicUpdate = millis();
          if (ULTRASONIC_ERROR_THRESHOLD < distance) {
            writeToLog("system.csv", String(millis()) + ",STATE,ARRIVED,0,Arrived at target location - distance to target: " + String(distance, 2) + " cm");
            found = true;
            break;
          }
          if (searchTimes % 2 == 0) {
            motorControl("right", searchTimes * 100); // Turn right
          } else {
            motorControl("left", searchTimes * 100); // Turn left
          }
        }
        if (found) {
          motorControl("forward", 100); // Move forward slowly if not found
        } else {
          motorControl("stop"); // Stop if not found after search
          state = "arrived"; // Go back to arrived state
        }
  } else if (state == "arrived") {
    motorControl("stop"); // Stop all motors
  } else {
    motorControl("stop"); // Default to stopping motors in unknown states
  }

  //Check End condition
  if (state == "arrived") {
    motorControl("stop"); // Stop all motors
    state = "standby";
    state_description = "Mission complete, system in standby";
    while (true) {
      ledMessage("standby");
      delay(1000);
    }
  }
  

  delay(100); // debounce
}
