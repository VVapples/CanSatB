#include "gps.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include "sd_logger.h"



HardwareSerial* gpsSerial = nullptr;
TinyGPSPlus gps;

static int GpsBoundRate = 9600;

static int lastDataReceived = 0;
static bool moduleDetected = false;
static bool newData = false;

bool setupGps(int txPin, int rxPin) {
    if (txPin == 9 && rxPin == 10) {
        gpsSerial = new HardwareSerial(1); // Use Serial1
        gpsSerial->begin(GpsBoundRate, SERIAL_8N1, rxPin, txPin);
    } else if (txPin == 16 && rxPin == 17) {
        gpsSerial = new HardwareSerial(2); // Use Serial2
        gpsSerial->begin(GpsBoundRate, SERIAL_8N1, rxPin, txPin);
    } else if (txPin == 1 && rxPin == 3) {
      // CANNOT USE SERIAL BUS TO SEE LOGS
        gpsSerial = new HardwareSerial(0); // Use Serial0
        gpsSerial->begin(GpsBoundRate, SERIAL_8N1, rxPin, txPin);
    } else {
      return false; // Invalid pin combination
    }
    writeLogHeaders("gps_data.csv", "Timestamp,HasFix,Latitude,Longitude,Altitude,Satellites,FixQuality,FixType,HDOP,Speed,Course,Date,Time,TimeValid");
    return true;
}

bool updateGps() {
  int loopCount = 0;
    while (gpsSerial->available() > 0 && loopCount < 500) {
        char c = gpsSerial->read();
        if (gps.encode(c)) {
          lastDataReceived = millis();
          moduleDetected = true;
          newData = true;
        }
        yield(); // Let watchdog timer reset
        loopCount++;
    }
    return newData;
}

GpsData getGpsData() {
  GpsData data;

  data.moduleDetected = moduleDetected;
  data.lastUpdate = lastDataReceived;

  if (newData) {
    newData = false;

    data.hasFix = gps.location.isValid();
    data.latitude = gps.location.lat();
    data.longitude = gps.location.lng();
    data.altitude = gps.altitude.meters();
    data.satelliteCount = gps.satellites.value();
    data.fixQuality = static_cast<int>(gps.location.FixQuality());
    data.fixType = (data.fixQuality >= 1) ? ((data.hasFix && gps.altitude.isValid()) ? 3 : 2) : 1; // Simplified fix type
    data.hdop = gps.hdop.hdop();
    data.speed = gps.speed.kmph();
    data.course = gps.course.deg();
    data.date = gps.date.value();
    data.time = gps.time.value();
    data.timeValid = gps.date.isValid() && gps.time.isValid();
  } else {
    // If no new data, return invalid/default values
    data.hasFix = false;
    data.latitude = 0.0;
    data.longitude = 0.0;
    data.altitude = 0.0;
    data.satelliteCount = 0;
    data.fixQuality = 0;
    data.fixType = 1; // No fix
    data.hdop = 0.0;
    data.speed = 0.0;
    data.course = 0.0;
    data.date = 0;
    data.time = 0;
    data.timeValid = false;
  }

  writeToLog("gps_data.csv", String(millis()) + "," +
               String(data.hasFix) + "," +
               String(data.latitude, 6) + "," +
               String(data.longitude, 6) + "," +
               String(data.altitude, 2) + "," +
               String(data.satelliteCount) + "," +
               String(data.fixQuality) + "," +
               String(data.fixType) + "," +
               String(data.hdop, 2) + "," +
               String(data.speed, 2) + "," +
               String(data.course, 2) + "," +
               String(data.date) + "," +
               String(data.time) + "," +
               String(data.timeValid ? 1 : 0));
               
  return data;
}