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

void setup() {
  Serial.begin(115200);
  if (!setupSdLogger(SD_CD_PIN)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized successfully.");

  // Write headers for the log files
  writeLogHeaders("gps.csv", "Time,Latitude,Longitude");
  writeLogHeaders("ultrasonic.csv", "Time,Distance");
}

void loop() {
  // Example data to log
  String gpsData = "1234567890,37.7749,-122.4194";
  String ultrasonicData = "1234567890,150";

  // Write GPS data to log
  writeToLog("gps.csv", gpsData);

  // Write Ultrasonic data to log
  writeToLog("ultrasonic.csv", ultrasonicData);

  delay(1000);  // Log data every second
}
