#include <Arduino.h>
#include "all.h"

//pins SETLATER
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define GPS_COMMS_CONFIG SERIAL_8N1
#define ULTRASONIC_TRIGGER_PIN 18
#define ULTRASONIC_ECHO_PIN 19

// put function declarations here:
int myFunction(int, int);

void setup() {
  //Serial w/ pc
  Serial.begin(115200);

  //scr
  
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}