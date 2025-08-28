#include <Arduino.h>
#include "gps.h"

bool gpsDetected = false;

void setup() {
  delay(2000); // Give some time for the Serial monitor to start
  Serial.begin(9600);
  setupGps(12, 13);
  Serial.println("Starting GPS test...");
}

void loop() {
  if (updateGps()) {
    gpsDetected = true;
    GpsData data = getGpsData();
    if (data.hasFix) {
      Serial.print("Lat: ");
      Serial.print(data.latitude, 6);
      Serial.print(", Lon: ");
      Serial.print(data.longitude, 6);
      Serial.print(", Alt: ");
      Serial.print(data.altitude, 2);
      Serial.print("m, Sats: ");
      Serial.print(data.satelliteCount);
      Serial.print(", HDOP: ");
      Serial.println(data.hdop, 2);
    } else {
      Serial.println("No GPS fix.");
    }
  } else {
    if (!gpsDetected) {
      Serial.println("Waiting for GPS data... (Is GPS module connected?)");
      delay(2000); // Print every 2 seconds if not detected
    }
  }
  delay(1000);
}