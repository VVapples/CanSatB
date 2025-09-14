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
  state = "setup";
  Serial.begin(115200);
  Serial.println("starting");

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
  writeToLog("system.csv", String(millis()) + ",GPS,INIT_BEGIN,0,Starting GPS initialization");
  setupGps(GPS_TX_PIN, GPS_RX_PIN);
  configureGps(1, 9600); // 1 Hz update rate, 9600 baud
  delay(1000); // Give some time for GPS to start up
  if (!updateGps()) {
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
  
  // writeToLog("system.csv", String(millis()) + ",GPS,INIT_SUCCESS,0,GPS initializeing begin");
  // setupGps(GPS_TX_PIN, GPS_RX_PIN);
  // writeToLog("system.csv", String(millis()) + ",GPS,INIT_SUCCESS,0,GPS initialized successfully");
  // configureGps(1, 9600); // 1 Hz update rate, 9600 baud
  // writeToLog("system.csv", String(millis()) + ",GPS,CONFIG_SUCCESS,0,GPS configured successfully");

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

  static unsigned long lastLogTime = 0;
  if (millis() - lastLogTime >= 5000) {
    writeToLog("system.csv", String(millis()) + ",SYSTEM,LOOP_RUNNING,0,System main loop running - State: " + state + ", Description: " + state_description);
    lastLogTime = millis();
  }

  // // Update sensor data
  // updateBno055Data();
  updateGps();
  // float distance = getDistanceCm();

  // // logs are made within other codes indivisulally
  // // no need to log here

  //debugging

  writeLogHeaders("gps_data.csv", "timestamp,latitude,longitude,altitude,hasFix,moduleDetected,satelliteCount,hdop,fixQuality,fixType,speed,course,timeValid,date,time,lastUpdate");
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

  
  delay(100); // Adjust delay as needed for your application
}
