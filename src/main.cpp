#include <Arduino.h>
#include "gps.h"

void setup() {
  Serial.begin(9600);
  // Set your GPS module's TX and RX pins here (example: TX=4, RX=3)
  setupGps(4, 3);
}

void loop() {
  if (updateGps()) {
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
  }
  delay(1000); // Print every second
}