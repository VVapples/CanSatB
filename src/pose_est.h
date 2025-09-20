#ifndef POSE_EST_H
#define POSE_EST_H

#include <Arduino.h>
#include "gps.h"
#include "9axis.h"

// EKF State structure
struct EKFState {
  float x;         // Position X (meters)
  float y;         // Position Y (meters)
  float vx;        // Velocity X (m/s)
  float vy;        // Velocity Y (m/s)
  float theta;     // Heading (radians)
  float omega;     // Angular velocity (rad/s)
};

// Enhanced pose structure with EKF estimates
struct Pose {
  float latitude;     // Estimated latitude in degrees
  float longitude;    // Estimated longitude in degrees
  float heading;      // Estimated heading in degrees (0-360)
  float speed;        // Estimated speed in m/s
  float x;           // Local X position in meters
  float y;           // Local Y position in meters
  float confidence;   // Estimation confidence (0-1)
};

// EKF Class for pose estimation
class ExtendedKalmanFilter {
private:
  EKFState state;           // Current state estimate
  float P[6][6];           // Covariance matrix (6x6)
  float Q[6][6];           // Process noise covariance
  float dt;                // Time step
  unsigned long lastUpdate; // Last update timestamp
  
  // Reference GPS coordinates for local frame
  float ref_lat;
  float ref_lon;
  bool ref_set;
  
  // Matrix operations
  void matrixMultiply(float A[][6], float B[][6], float result[][6], int size);
  void matrixAdd(float A[][6], float B[][6], float result[][6], int size);
  void matrixTranspose(float A[][6], float result[][6], int rows, int cols);
  void matrixInvert2x2(float A[2][2], float result[2][2]);
  
  // Coordinate conversions
  void gpsToLocal(float lat, float lon, float* x, float* y);
  void localToGps(float x, float y, float* lat, float* lon);
  
public:
  ExtendedKalmanFilter();
  
  /**
   * @brief Initialize the EKF with initial state
   * @param initial_lat Initial latitude
   * @param initial_lon Initial longitude
   * @param initial_heading Initial heading in degrees
   */
  void initialize(float initial_lat, float initial_lon, float initial_heading);
  
  /**
   * @brief Predict step of EKF using IMU data
   * @param accel_x Linear acceleration X (m/s²)
   * @param accel_y Linear acceleration Y (m/s²)
   * @param gyro_z Angular velocity Z (rad/s)
   * @param dt_ms Time step in milliseconds
   */
  void predict(float accel_x, float accel_y, float gyro_z, float dt_ms);
  
  /**
   * @brief Update step of EKF using GPS measurements
   * @param gps_lat GPS latitude
   * @param gps_lon GPS longitude
   * @param gps_valid GPS measurement validity
   */
  void updateGPS(float gps_lat, float gps_lon, bool gps_valid);
  
  /**
   * @brief Update step of EKF using magnetometer heading
   * @param mag_heading Magnetometer heading in degrees
   * @param mag_valid Magnetometer measurement validity
   */
  void updateHeading(float mag_heading, bool mag_valid);
  
  /**
   * @brief Get current pose estimate
   * @return Pose structure with all estimated values
   */
  Pose getPoseEstimate();
  
  /**
   * @brief Get current state for debugging
   * @return EKFState structure
   */
  EKFState getState() { return state; }
};

// Global EKF instance
extern ExtendedKalmanFilter ekf;

/**
 * @brief Initialize the EKF pose estimation system
 * Call this once during system setup
 */
void initializePoseEstimation();

/**
 * @brief Update pose estimation with new sensor data
 * @param gpsData Current GPS data
 * @param bno055Data Current BNO055 sensor data
 * @return Updated pose estimate
 */
Pose updatePoseEstimation(const GpsData& gpsData, const Bno055Data& bno055Data);

/**
 * @brief Sets up CSV logging headers for EKF pose estimation data
 */
void setupPoseLogging();

/**
 * @brief Calculate target direction from current position to target position
 * @param current_lat Current latitude in degrees
 * @param current_lon Current longitude in degrees
 * @param target_lat Target latitude in degrees
 * @param target_lon Target longitude in degrees
 * @return Direction to target in degrees (0-360)
 */
float estimateTargetDirection(float current_lat, float current_lon, float target_lat, float target_lon);

/**
 * @brief Calculate distance to target position
 * @param current_lat Current latitude in degrees
 * @param current_lon Current longitude in degrees
 * @param target_lat Target latitude in degrees
 * @param target_lon Target longitude in degrees
 * @return Distance to target in meters
 */
float calculateDistanceToTarget(float current_lat, float current_lon, float target_lat, float target_lon);

/**
 * @brief Get target position from SD card file
 * @return PositionData structure with target coordinates
 */
struct PositionData {
  float latitude;
  float longitude;
};

PositionData getTargetPosition();

#endif // POSE_EST_H