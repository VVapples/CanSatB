#include <Arduino.h>
#include "9axis.h"
#include "gps.h"
#include "sd_logger.h"
#include "ultrasonic.h"


//pins: all in gpio pin numbers
#define GPS_RX_PIN 10
#define GPS_TX_PIN 9
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
  state = "setup";

  //SDcard setup : if failed ed with s
  if (!setupSdLogger(SD_CD_PIN)) {
    state = "error";
    state_description = "SD Card initialization failed!";
    writeToLog("system.csv", String(millis()) + ",SYSTEM,INIT_FAILED,-1,SD Card initialization failed!");
    while (true) {
      // Stay here forever if SD card fails to initialize
      delay(1000);
    }
  } else {
    writeToLog("system.csv", String(millis()) + ",SYSTEM,INIT_SUCCESS,0,SD Card initialized successfully");
  }

  //other sensor setups
  if (!setupBno055()) {
    state = "error";
    state_description = "BNO055 initialization failed!";
    writeToLog("system.csv", String(millis()) + ",BNO055,INIT_FAILED,-1,BNO055 initialization failed!");
    while (true) {
      // Stay here forever if BNO055 fails to initialize
      delay(1000);
    }
  } else {
    writeToLog("system.csv", String(millis()) + ",BNO055,INIT_SUCCESS,0,BNO055 initialized successfully");
  }
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
  }
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

  // Log system startup completion
  state = "ready";
  state_description = "All systems initialized successfully";
  writeToLog("system.csv", String(millis()) + ",SYSTEM,STARTUP_COMPLETE,0,All sensors initialized and system ready");
}

void loop() {
  state = "running";
  // Update sensor data
  updateBno055Data();
  updateGps();
  float distance = getDistanceCm();

  // logs are made within other codes indivisulally
  // no need to log here

  delay(100); // Adjust delay as needed for your application
}
