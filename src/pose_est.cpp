#include "pose_est.h"
#include <Arduino.h>
#include "sd_logger.h"

// Implementation of getCurrentPose function
Pose getCurrentPose(const GpsData& gpsData, const Bno055Data& bno055Data) {
  Pose currentPose;
  bool gpsValid = false;
  bool bnoValid = false;
  
  // GPS Position Processing - Enhanced validation for default values
  if (gpsData.hasFix && gpsData.moduleDetected && 
      gpsData.latitude != 0.0 && gpsData.longitude != 0.0 &&
      gpsData.hdop < 20.0) {  // Good HDOP indicates reliable GPS signal
    
    // Valid GPS data - use actual coordinates
    currentPose.latitude = gpsData.latitude;
    currentPose.longitude = gpsData.longitude;
    gpsValid = true;
    
  } else {
    // GPS invalid, no fix, or default values - indicate no valid position
    currentPose.latitude = 0.0;
    currentPose.longitude = 0.0;
    gpsValid = false;
  }
  
  // BNO055 Heading Processing - Handle default quaternion and heading values
  if (bno055Data.quatW != 1.0 || bno055Data.quatX != 0.0 || 
      bno055Data.quatY != 0.0 || bno055Data.quatZ != 0.0) {
    
    // BNO055 has moved from default identity quaternion - use heading
    currentPose.heading = bno055Data.heading;
    
    // Normalize heading to 0-360 range efficiently
    currentPose.heading = fmod(currentPose.heading, 360.0);
    if (currentPose.heading < 0.0) {
      currentPose.heading += 360.0;
    }
    bnoValid = true;
    
  } else {
    // BNO055 still at default values - assume 0° heading (North)
    currentPose.heading = 0.0;
    bnoValid = false;
  }
  
  // Log the new pose data
  String poseLogData = String(millis()) + "," +
                       String(currentPose.latitude, 7) + "," +
                       String(currentPose.longitude, 7) + "," +
                       String(currentPose.heading, 2) + "," +
                       String(gpsValid ? 1 : 0) + "," +
                       String(bnoValid ? 1 : 0);
  
  writeToLog("pose_data.csv", poseLogData);
  
  return currentPose;
}

// Implementation of setupPoseLogging function
void setupPoseLogging() {
  // Setup CSV headers for pose estimation logging
  writeLogHeaders("pose_data.csv", "timestamp,latitude,longitude,heading,gps_valid,bno_valid");
}
