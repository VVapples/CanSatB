#include "pose_est.h"
#include <Arduino.h>
#include "sd_logger.h"
#include "calculations.h"
#include "calculations.h"

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
  
  // BNO055 Magnetometer Heading Processing - Use raw magnetometer data
  if ((bno055Data.magX != 0.0 || bno055Data.magY != 0.0) &&
      (bno055Data.quatW != 1.0 || bno055Data.quatX != 0.0 || 
       bno055Data.quatY != 0.0 || bno055Data.quatZ != 0.0)) {
    
    // BNO055 has valid magnetometer data - calculate heading from raw mag data
    currentPose.heading = calculateMagneticHeading(bno055Data.magX, bno055Data.magY);
    bnoValid = true;
    
  } else {
    // BNO055 still at default values or no mag data - assume 0° heading (North)
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
