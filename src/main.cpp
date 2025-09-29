#include <Arduino.h>
#include "gps.h"

bool gpsDetected = false;
GpsData currentGpsData;
uint32_t lastStatusCheck = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting GPS Module...");
  setupGps(16, 17); // Example pins for TX and RX
  Serial.println("GPS Module Initialized");
}

void loop() {
  if (updateGps()) {
    Serial.println("GPS data received.");
    currentGpsData = getGpsData();
    Serial.print("Latitude: ");
    Serial.println(currentGpsData.latitude, 6);
    Serial.print("Longitude: ");
    Serial.println(currentGpsData.longitude, 6);
    
  } else {
    Serial.println("No GPS data received.");
  }
  delay(1000); // Check GPS status every second
}