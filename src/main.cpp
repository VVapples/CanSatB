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
  setupSdLogger(SD_CD_PIN);
  if (!setupSdLogger(SD_CD_PIN)) {
    state = "error";
    state_description = "SD Card initialization failed!";
    writeToLog("system.csv", String(millis()) + ",SYSTEM,INIT_FAILED,-1,SD Card initialization failed!");
    while (true) {
      // Stay here forever if SD card fails to initialize
      delay(1000);
    }
  }

  //other sensor setups
  setupGps(GPS_TX_PIN, GPS_RX_PIN);
  setupUltrasonic(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN);
  setupMpu9250();
  if (!setupMpu9250()) {
    state = "error";
    state_description = "MPU9250 initialization failed!";
    writeToLog("system.csv", String(millis()) + ",MPU9250,INIT_FAILED,-1,MPU9250 initialization failed!");
    while (true) {
      // Stay here forever if MPU9250 fails to initialize
      delay(1000);
    }
  }
  if (!setupGps(GPS_TX_PIN, GPS_RX_PIN)) {
    state = "error";
    state_description = "GPS initialization failed!";
    writeToLog("system.csv", String(millis()) + ",GPS,INIT_FAILED,-1,GPS initialization failed!");
    while (true) {
      // Stay here forever if GPS fails to initialize
      delay(1000);
    }
  }
  if (!setupUltrasonic(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN)) {
    state = "error";
    state_description = "Ultrasonic sensor initialization failed!";
    writeToLog("system.csv", String(millis()) + ",ULTRASONIC,INIT_FAILED,-1,Ultrasonic sensor initialization failed!");
    while (true) {
      // Stay here forever if Ultrasonic sensor fails to initialize
      delay(1000);
    }
  }
}

void loop() {
  state = "running";
  // Update sensor data
  updateMpu9250Data();
  updateGps();
  float distance = getDistanceCm();

  // logs are made within other codes indivisulally
  // no need to log here

  delay(100); // Adjust delay as needed for your application
}
