#include <Arduino.h>
#include "9axis.h"
#include "gps.h"
#include "sd_logger.h"
#include "ultrasonic.h"


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

void setup() {
  // Note: Serial removed to free up UART for other modules
  state = "setup";

  //SDcard setup : if failed with errors
  if (!setupSdLogger(SD_CD_PIN)) {
    state = "error";
    state_description = "SD Card initialization failed!";
    // Can't log to SD if SD failed, so halt
    while (true) {
      delay(1000);
    }
  } else {
    // setup logging headers
    writeLogHeaders("system.csv", "timestamp,state,code,message");
    writeLogHeaders("debug.csv", "timestamp,module,event,code,message");
    
    writeToLog("system.csv", String(millis()) + ",SD,INIT_SUCCESS,0,SD card initialized successfully");
    writeToLog("debug.csv", String(millis()) + ",MAIN,SETUP_START,0,CanSat System Starting");
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

  // GPS setup with crash protection
  writeToLog("debug.csv", String(millis()) + ",MAIN,GPS_INIT_START,0,Starting GPS initialization");
  
  try {
    bool gpsResult = setupGps(GPS_TX_PIN, GPS_RX_PIN);
    writeToLog("debug.csv", String(millis()) + ",MAIN,GPS_SETUP_RESULT,0,GPS setup result: " + String(gpsResult ? "SUCCESS" : "FAILED"));
    
    if (!gpsResult) {
      state = "error";
      state_description = "GPS initialization failed!";
      writeToLog("system.csv", String(millis()) + ",GPS,INIT_FAILED,-1,GPS initialization failed - no module detected");
      writeToLog("debug.csv", String(millis()) + ",MAIN,GPS_HALT,0,GPS failed - system will halt");
      while (true) {
        // Stay here forever if GPS fails to initialize
        delay(1000);
      }
    } else {
      writeToLog("system.csv", String(millis()) + ",GPS,INIT_SUCCESS,0,GPS initialized successfully");
      writeToLog("debug.csv", String(millis()) + ",MAIN,GPS_SUCCESS,0,GPS initialization completed successfully");
    }
  } catch (...) {
    state = "error";
    state_description = "GPS initialization crashed!";
    writeToLog("system.csv", String(millis()) + ",GPS,INIT_CRASHED,-2,GPS initialization caused system crash");
    writeToLog("debug.csv", String(millis()) + ",MAIN,GPS_CRASH,0,GPS initialization caused a crash!");
    while (true) {
      delay(1000);
    }
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
  // Add debug output to track loop execution
  static uint32_t lastDebugOutput = 0;
  if (millis() - lastDebugOutput > 5000) { // Every 5 seconds
    writeToLog("system.csv", String(millis()) + ",LOOP,RUNNING,0,Main loop executing");
    writeToLog("debug.csv", String(millis()) + ",MAIN,LOOP_ALIVE,0,Loop running - system operational");
    lastDebugOutput = millis();
  }
  
  // Safely update GPS with error handling
  try {
    updateGps();
  } catch (...) {
    writeToLog("system.csv", String(millis()) + ",GPS,UPDATE_ERROR,-1,Exception in GPS update");
    writeToLog("debug.csv", String(millis()) + ",MAIN,GPS_UPDATE_ERROR,0,Exception in updateGps() - continuing");
  }

  delay(100); // Adjust delay as needed for your application
}
