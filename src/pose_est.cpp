#include "pose_est.h"
#include <Arduino.h>
#include "sd_logger.h"
#include <math.h>

// Constants
#define DEG_TO_RAD 0.017453292519943295
#define RAD_TO_DEG 57.295779513082323
#define PI_CONST 3.14159265359
#define EARTH_RADIUS 6371000.0

// =============================================================================
// TUNABLE EKF PARAMETERS - ADJUST THESE FOR YOUR CANSAT
// =============================================================================

// Initial Uncertainty Values (how uncertain we are at startup)
const float INITIAL_POS_UNCERTAINTY = 100.0;    // Position uncertainty (m²) - Higher = less trust in initial position
const float INITIAL_VEL_UNCERTAINTY = 10.0;     // Velocity uncertainty (m²/s²) - Higher = less trust in initial velocity
const float INITIAL_HEADING_UNCERTAINTY = 1.0;  // Heading uncertainty (rad²) - Higher = less trust in initial heading
const float INITIAL_ANGVEL_UNCERTAINTY = 0.1;   // Angular velocity uncertainty (rad²/s²)

// Process Noise Values (how much the system changes between measurements)
const float POS_PROCESS_NOISE = 0.1;     // Position process noise - Higher = more responsive to position changes
const float VEL_PROCESS_NOISE = 1.0;     // Velocity process noise - Higher = more responsive to acceleration
const float HEADING_PROCESS_NOISE = 0.01; // Heading process noise - Higher = more responsive to heading changes
const float ANGVEL_PROCESS_NOISE = 0.1;  // Angular velocity process noise

// Measurement Noise Values (how noisy our sensors are)
const float GPS_POSITION_NOISE = 25.0;   // GPS position uncertainty (m²) - Decrease for better GPS (1-9 for good GPS)
const float HEADING_UPDATE_GAIN = 0.1;   // Magnetometer heading gain (0-1) - Higher = more responsive heading

// GPS Quality Thresholds
const float GPS_HDOP_THRESHOLD = 20.0;   // GPS quality threshold - Lower = stricter GPS requirements (1-5 for good GPS)

// Confidence Calculation
const float CONFIDENCE_SCALE = 100.0;    // Confidence scaling factor - Lower = more sensitive confidence

// =============================================================================

// Global EKF instance
ExtendedKalmanFilter ekf;

// EKF Constructor
ExtendedKalmanFilter::ExtendedKalmanFilter() {
  // Initialize state to zero
  state.x = 0.0;
  state.y = 0.0;
  state.vx = 0.0;
  state.vy = 0.0;
  state.theta = 0.0;
  state.omega = 0.0;
  
  // Initialize covariance matrix P (uncertainty)
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 6; j++) {
      P[i][j] = 0.0;
      Q[i][j] = 0.0;
    }
  }
  
  // Initial uncertainty (diagonal elements)
  P[0][0] = INITIAL_POS_UNCERTAINTY;  // Position X uncertainty (m²)
  P[1][1] = INITIAL_POS_UNCERTAINTY;  // Position Y uncertainty (m²)
  P[2][2] = INITIAL_VEL_UNCERTAINTY;   // Velocity X uncertainty (m²/s²)
  P[3][3] = INITIAL_VEL_UNCERTAINTY;   // Velocity Y uncertainty (m²/s²)
  P[4][4] = INITIAL_HEADING_UNCERTAINTY;    // Heading uncertainty (rad²)
  P[5][5] = INITIAL_ANGVEL_UNCERTAINTY;    // Angular velocity uncertainty (rad²/s²)
  
  // Process noise covariance Q
  Q[0][0] = POS_PROCESS_NOISE;    // Position X process noise
  Q[1][1] = POS_PROCESS_NOISE;    // Position Y process noise
  Q[2][2] = VEL_PROCESS_NOISE;    // Velocity X process noise
  Q[3][3] = VEL_PROCESS_NOISE;    // Velocity Y process noise
  Q[4][4] = HEADING_PROCESS_NOISE;   // Heading process noise
  Q[5][5] = ANGVEL_PROCESS_NOISE;    // Angular velocity process noise
  
  ref_set = false;
  lastUpdate = 0;
}

void ExtendedKalmanFilter::initialize(float initial_lat, float initial_lon, float initial_heading) {
  ref_lat = initial_lat;
  ref_lon = initial_lon;
  ref_set = true;
  
  state.x = 0.0;
  state.y = 0.0;
  state.vx = 0.0;
  state.vy = 0.0;
  state.theta = initial_heading * DEG_TO_RAD;
  state.omega = 0.0;
  
  lastUpdate = millis();
}

void ExtendedKalmanFilter::predict(float accel_x, float accel_y, float gyro_z, float dt_ms) {
  dt = dt_ms / 1000.0; // Convert to seconds
  
  if (dt <= 0 || dt > 1.0) return; // Sanity check
  
  // State prediction (motion model)
  float cos_theta = cos(state.theta);
  float sin_theta = sin(state.theta);
  
  // Convert accelerations from body frame to world frame
  float ax_world = accel_x * cos_theta - accel_y * sin_theta;
  float ay_world = accel_x * sin_theta + accel_y * cos_theta;
  
  // Update state using motion model
  state.x += state.vx * dt + 0.5 * ax_world * dt * dt;
  state.y += state.vy * dt + 0.5 * ay_world * dt * dt;
  state.vx += ax_world * dt;
  state.vy += ay_world * dt;
  state.theta += state.omega * dt;
  state.omega = gyro_z; // Assume angular velocity measurement
  
  // Normalize heading
  while (state.theta > PI_CONST) state.theta -= 2 * PI_CONST;
  while (state.theta < -PI_CONST) state.theta += 2 * PI_CONST;
  
  // Jacobian of motion model (F matrix)
  float F[6][6] = {{0}};
  F[0][0] = 1.0; F[0][2] = dt;
  F[1][1] = 1.0; F[1][3] = dt;
  F[2][2] = 1.0;
  F[3][3] = 1.0;
  F[4][4] = 1.0; F[4][5] = dt;
  F[5][5] = 1.0;
  
  // Predict covariance: P = F*P*F' + Q
  float temp[6][6], FT[6][6];
  matrixTranspose(F, FT, 6, 6);
  matrixMultiply(F, P, temp, 6);
  matrixMultiply(temp, FT, P, 6);
  matrixAdd(P, Q, P, 6);
}

void ExtendedKalmanFilter::updateGPS(float gps_lat, float gps_lon, bool gps_valid) {
  if (!gps_valid || !ref_set) return;
  
  // Convert GPS to local coordinates
  float measured_x, measured_y;
  gpsToLocal(gps_lat, gps_lon, &measured_x, &measured_y);
  
  // Measurement model: z = H*x + v
  // We measure position directly
  float H[2][6] = {{1, 0, 0, 0, 0, 0},
                   {0, 1, 0, 0, 0, 0}};
  
  // Measurement noise covariance R
  float R[2][2] = {{GPS_POSITION_NOISE, 0.0},    // GPS position uncertainty
                   {0.0, GPS_POSITION_NOISE}};
  
  // Innovation (measurement residual)
  float y[2] = {measured_x - state.x, measured_y - state.y};
  
  // Innovation covariance S = H*P*H' + R
  float S[2][2];
  S[0][0] = P[0][0] + R[0][0];
  S[0][1] = P[0][1] + R[0][1];
  S[1][0] = P[1][0] + R[1][0];
  S[1][1] = P[1][1] + R[1][1];
  
  // Kalman gain K = P*H'*inv(S)
  float S_inv[2][2];
  matrixInvert2x2(S, S_inv);
  
  float K[6][2];
  for (int i = 0; i < 6; i++) {
    K[i][0] = P[i][0] * S_inv[0][0] + P[i][1] * S_inv[1][0];
    K[i][1] = P[i][0] * S_inv[0][1] + P[i][1] * S_inv[1][1];
  }
  
  // Update state: x = x + K*y
  state.x += K[0][0] * y[0] + K[0][1] * y[1];
  state.y += K[1][0] * y[0] + K[1][1] * y[1];
  state.vx += K[2][0] * y[0] + K[2][1] * y[1];
  state.vy += K[3][0] * y[0] + K[3][1] * y[1];
  state.theta += K[4][0] * y[0] + K[4][1] * y[1];
  state.omega += K[5][0] * y[0] + K[5][1] * y[1];
  
  // Update covariance: P = (I - K*H)*P
  float I_KH[6][6];
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 6; j++) {
      I_KH[i][j] = (i == j) ? 1.0 : 0.0;
      if (j < 2) {
        I_KH[i][j] -= K[i][j] * H[j][i];
      }
    }
  }
  
  float temp[6][6];
  matrixMultiply(I_KH, P, temp, 6);
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 6; j++) {
      P[i][j] = temp[i][j];
    }
  }
}

void ExtendedKalmanFilter::updateHeading(float mag_heading, bool mag_valid) {
  if (!mag_valid) return;
  
  float measured_theta = mag_heading * DEG_TO_RAD;
  
  // Handle angle wrapping
  float angle_diff = measured_theta - state.theta;
  while (angle_diff > PI_CONST) angle_diff -= 2 * PI_CONST;
  while (angle_diff < -PI_CONST) angle_diff += 2 * PI_CONST;
  
  // Simple heading update with configurable gain
  state.theta += HEADING_UPDATE_GAIN * angle_diff;
  
  // Normalize heading
  while (state.theta > PI_CONST) state.theta -= 2 * PI_CONST;
  while (state.theta < -PI_CONST) state.theta += 2 * PI_CONST;
}

Pose ExtendedKalmanFilter::getPoseEstimate() {
  Pose pose;
  
  if (ref_set) {
    localToGps(state.x, state.y, &pose.latitude, &pose.longitude);
  } else {
    pose.latitude = 0.0;
    pose.longitude = 0.0;
  }
  
  pose.heading = state.theta * RAD_TO_DEG;
  if (pose.heading < 0) pose.heading += 360.0;
  
  pose.speed = sqrt(state.vx * state.vx + state.vy * state.vy);
  pose.x = state.x;
  pose.y = state.y;
  
  // Calculate confidence based on trace of covariance matrix
  float trace = P[0][0] + P[1][1] + P[4][4];
  pose.confidence = exp(-trace / CONFIDENCE_SCALE); // Configurable confidence scaling
  
  return pose;
}

// Coordinate conversion functions
void ExtendedKalmanFilter::gpsToLocal(float lat, float lon, float* x, float* y) {
  float dlat = (lat - ref_lat) * DEG_TO_RAD;
  float dlon = (lon - ref_lon) * DEG_TO_RAD;
  
  *x = dlon * EARTH_RADIUS * cos(ref_lat * DEG_TO_RAD);
  *y = dlat * EARTH_RADIUS;
}

void ExtendedKalmanFilter::localToGps(float x, float y, float* lat, float* lon) {
  *lat = ref_lat + (y / EARTH_RADIUS) * RAD_TO_DEG;
  *lon = ref_lon + (x / (EARTH_RADIUS * cos(ref_lat * DEG_TO_RAD))) * RAD_TO_DEG;
}

// Matrix operations
void ExtendedKalmanFilter::matrixMultiply(float A[][6], float B[][6], float result[][6], int size) {
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      result[i][j] = 0.0;
      for (int k = 0; k < size; k++) {
        result[i][j] += A[i][k] * B[k][j];
      }
    }
  }
}

void ExtendedKalmanFilter::matrixAdd(float A[][6], float B[][6], float result[][6], int size) {
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      result[i][j] = A[i][j] + B[i][j];
    }
  }
}

void ExtendedKalmanFilter::matrixTranspose(float A[][6], float result[][6], int rows, int cols) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      result[j][i] = A[i][j];
    }
  }
}

void ExtendedKalmanFilter::matrixInvert2x2(float A[2][2], float result[2][2]) {
  float det = A[0][0] * A[1][1] - A[0][1] * A[1][0];
  if (abs(det) < 1e-6) {
    // Singular matrix, use identity
    result[0][0] = 1.0; result[0][1] = 0.0;
    result[1][0] = 0.0; result[1][1] = 1.0;
    return;
  }
  
  result[0][0] = A[1][1] / det;
  result[0][1] = -A[0][1] / det;
  result[1][0] = -A[1][0] / det;
  result[1][1] = A[0][0] / det;
}

// Global functions
void initializePoseEstimation() {
  setupPoseLogging();
}

Pose updatePoseEstimation(const GpsData& gpsData, const Bno055Data& bno055Data) {
  static bool initialized = false;
  static unsigned long lastTime = 0;
  
  unsigned long currentTime = millis();
  
  // Initialize EKF with first valid GPS reading
  if (!initialized && gpsData.hasFix && gpsData.moduleDetected && 
      gpsData.latitude != 0.0 && gpsData.longitude != 0.0) {
    ekf.initialize(gpsData.latitude, gpsData.longitude, bno055Data.heading);
    initialized = true;
    lastTime = currentTime;
  }
  
  if (!initialized) {
    // Return default pose if not initialized
    Pose pose = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    return pose;
  }
  
  // Predict step with IMU data
  if (currentTime > lastTime) {
    float dt_ms = currentTime - lastTime;
    ekf.predict(bno055Data.linearAccelX, bno055Data.linearAccelY, 
                bno055Data.gyroZ * DEG_TO_RAD, dt_ms);
  }
  
  // Update step with GPS data
  bool gpsValid = gpsData.hasFix && gpsData.moduleDetected && 
                  gpsData.latitude != 0.0 && gpsData.longitude != 0.0 &&
                  gpsData.hdop < GPS_HDOP_THRESHOLD;
  
  if (gpsValid) {
    ekf.updateGPS(gpsData.latitude, gpsData.longitude, true);
  }
  
  // Update step with magnetometer heading
  bool magValid = (bno055Data.quatW != 1.0 || bno055Data.quatX != 0.0 || 
                   bno055Data.quatY != 0.0 || bno055Data.quatZ != 0.0);
  
  if (magValid) {
    ekf.updateHeading(bno055Data.heading, true);
  }
  
  lastTime = currentTime;
  
  // Get pose estimate
  Pose pose = ekf.getPoseEstimate();
  
  // Log the pose data
  String poseLogData = String(currentTime) + "," +
                       String(pose.latitude, 7) + "," +
                       String(pose.longitude, 7) + "," +
                       String(pose.heading, 2) + "," +
                       String(pose.speed, 2) + "," +
                       String(pose.x, 2) + "," +
                       String(pose.y, 2) + "," +
                       String(pose.confidence, 3) + "," +
                       String(gpsValid ? 1 : 0) + "," +
                       String(magValid ? 1 : 0);
  
  writeToLog("pose_data.csv", poseLogData);
  
  return pose;
}

void setupPoseLogging() {
  writeLogHeaders("pose_data.csv", "timestamp,latitude,longitude,heading,speed,x,y,confidence,gps_valid,mag_valid");
}
