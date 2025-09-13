#include <Arduino.h>
#include "9axis.h"
#include "gps.h"
#include "sd_logger.h"
#include "ultrasonic.h"
#include "pos_est.h"


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
static String state_description = "System is initializing";

//decider variables
static float maxDirectionOffset = 15; //degrees

void setup() {
  state = "setup";

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

  // // BNO055
  // if (!setupBno055()) {
  //   state = "error";
  //   state_description = "BNO055 initialization failed!";
  //   while (true) {
  //     // Stay here forever if BNO055 fails to initialize
  //     delay(1000);
  //   }
  // }

  // GPS
  if (!setupGps(GPS_TX_PIN, GPS_RX_PIN)) {
    state = "error";
    state_description = "GPS initialization failed!";
    writeToLog("system.csv", String(millis()) + ",GPS,INIT_FAILED,-1,GPS initialization failed!");
    while (true) {
      // Stay here forever if GPS fails to initialize
      delay(1000);
    }
  } else {
    writeToLog("system.csv", String(millis()) + ",GPS,INIT_SUCCESS,0,GPS initialized successfully");
    configureGps(1, 9600); // 1 Hz update rate, 9600 baud
  }
  
  // // Ultrasonic
  //
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

  // // Log system startup completion
  // state = "ready";
  // state_description = "All systems initialized successfully";
  // writeToLog("system.csv", String(millis()) + ",SYSTEM,STARTUP_COMPLETE,0,All sensors initialized and system ready");
}

void loop() {
  state = "running";

  // Loop logging
  static unsigned long lastLogTime = 0;
  if (millis() - lastLogTime >= 10000) {
    writeToLog("system.csv", String(millis()) + ",SYSTEM,LOOP_RUNNING,0,System main loop running - State: " + state + ", Description: " + state_description);
    lastLogTime = millis();
  }

  // // Update sensor data
  // updateBno055Data();
  updateGps();
  // float distance = getDistanceCm(); //ultrasonic

  // Position estimation
  PositionData currentPos = getPositionData();
  PositionData targetPos = getTargetPosition();
  float targetDirection = estimateTargetDirection(currentPos, targetPos);
  float distanceToTarget = calculateDistanceToTarget(currentPos, targetPos);
  
  // Decision making
  if (abs(targetDirection - currentPos.heading) > maxDirectionOffset) {
    // Take corrective action
  }

  // // logs are made within other codes indivisulally
  // // no need to log here

  //debugging

  GpsData currentGpsData = getGpsData();
  static String gpswriteBuffer = "";
  gpswriteBuffer = "" + String(millis()) + "," +
                   String(currentGpsData.latitude, 6) + "," +
                   String(currentGpsData.longitude, 6) + "," +
                   String(currentGpsData.altitude, 2) + "," +
                   String(currentGpsData.hasFix ? 1 : 0) + "," +
                   String(currentGpsData.moduleDetected ? 1 : 0) + "," +
                   String(currentGpsData.satelliteCount) + "," +
                   String(currentGpsData.hdop, 2) + "," +
                   String(currentGpsData.fixQuality) + "," +
                   String(currentGpsData.fixType) + "," +
                   String(currentGpsData.speed, 2) + "," +
                   String(currentGpsData.course, 2) + "," +
                   String(currentGpsData.timeValid ? 1 : 0) + "," +
                   String(currentGpsData.date) + "," +
                   String(currentGpsData.time) + "," +
                   String(currentGpsData.lastUpdate);
  writeToLog("gps_data.csv", gpswriteBuffer);

  //buffer
  delay(100);
}
