#include "pos_est.h"
#include <Arduino.h>
#include "gps.h"
#include "9axis.h"
#include <SD.h>
#include <math.h>

PositionData getPositionData() {
    GpsData gpsData = getGpsData();
    Bno055Data bnoData = getBno055Data();

    PositionData position;
    position.latitude = gpsData.latitude;
    position.longitude = gpsData.longitude;
    position.heading = bnoData.heading;

    return position;
}

// This function reads the target position from the SD card file `target_pos.csv`.
// The file is expected to contain a single line with the following structure:
// latitude,longitude
// Example: 37.7749,-122.4194
PositionData getTargetPosition() {
    PositionData targetPos;

    File file = SD.open("target_pos.csv", FILE_READ);
    if (file) {
        String line = file.readStringUntil('\n');
        int commaIndex = line.indexOf(',');

        if (commaIndex != -1) {
            targetPos.latitude = line.substring(0, commaIndex).toFloat();
            targetPos.longitude = line.substring(commaIndex + 1).toFloat();
            targetPos.heading = 0; // Heading is not part of the target position
        }

        file.close();
    } else {
        targetPos.latitude = 0;
        targetPos.longitude = 0;
        targetPos.heading = 0;
    }

    return targetPos;
}

// This function estimates the direction (in degrees) to the target position
// relative to the current position. The direction is similar to a heading.
float estimateTargetDirection(const PositionData& currentPos, const PositionData& targetPos) {
    float deltaY = targetPos.latitude - currentPos.latitude;
    float deltaX = targetPos.longitude - currentPos.longitude;
    float angle = atan2(deltaY, deltaX) * 180.0 / PI; // Convert radians to degrees

    // Ensure the angle is within the range [0, 360)
    if (angle < 0) {
        angle += 360.0;
    }

    return angle;
}

// This function calculates the distance (in meters) to the target position
// using the Haversine formula for great-circle distance.
float calculateDistanceToTarget(const PositionData& currentPos, const PositionData& targetPos) {
    const float R = 6371000; // Earth's radius in meters
    float lat1 = radians(currentPos.latitude);
    float lat2 = radians(targetPos.latitude);
    float deltaLat = radians(targetPos.latitude - currentPos.latitude);
    float deltaLon = radians(targetPos.longitude - currentPos.longitude);

    float a = sin(deltaLat / 2) * sin(deltaLat / 2) +
              cos(lat1) * cos(lat2) *
              sin(deltaLon / 2) * sin(deltaLon / 2);
    float c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return R * c;
}

