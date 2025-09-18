#ifndef POSE_EST_H
#define POSE_EST_H

#include <Arduino.h>
#include "gps.h"
#include "9axis.h"

// Simple pose structure containing only latitude, longitude, and heading
struct Pose {
  float latitude;     // Latitude in degrees from GPS
  float longitude;    // Longitude in degrees from GPS  
  float heading;      // Heading in degrees from BNO055 (0-360 degrees)
};

/**
 * @brief Gets current pose based on GPS and BNO055 sensor data.
 * This function combines GPS position data with BNO055 heading data to provide
 * a simple pose estimate containing latitude, longitude, and heading.
 * 
 * Optimizations for default values:
 * - GPS: Validates fix status, coordinates != 0.0, and HDOP < 20.0 for quality
 * - BNO055: Detects if quaternion has moved from identity (1,0,0,0) default
 * - Returns 0.0 coordinates when GPS invalid, 0.0° heading when BNO055 at defaults
 * 
 * @param gpsData Current GPS data structure
 * @param bno055Data Current BNO055 sensor data structure
 * @return Pose structure containing latitude, longitude, and heading
 */
Pose getCurrentPose(const GpsData& gpsData, const Bno055Data& bno055Data);

/**
 * @brief Sets up CSV logging headers for pose estimation data.
 * This function creates the header row for pose logging with columns:
 * timestamp, latitude, longitude, heading, gps_valid, bno_valid
 * Call this once during system initialization.
 */
void setupPoseLogging();

#endif // POSE_EST_H