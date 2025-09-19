# CanSat Project - Extended Kalman Filter (EKF) Pose Estimation

This CanSat project implements an advanced Extended Kalman Filter for accurate pose estimation using GPS and IMU sensor fusion.

## Overview

The system combines data from multiple sensors to provide optimal pose estimation:
- **GPS**: Provides position measurements (latitude/longitude)
- **BNO055 IMU**: Provides acceleration, gyroscope, and magnetometer data
- **EKF Algorithm**: Fuses sensor data for smooth, accurate pose estimates

## Hardware Components

- **GPS Module**: For position measurements
- **BNO055 9-axis IMU**: For motion and orientation data
- **SD Card**: For data logging
- **DRV8835 Motor Driver**: For movement control
- **ESP32/Arduino**: Main controller

## Quick Start

1. **Initialize the system**:
   ```cpp
   initializePoseEstimation();
   ```

2. **Update pose in main loop**:
   ```cpp
   GpsData gpsData = getGpsData();
   Bno055Data bno055Data = getBno055Data();
   Pose currentPose = updatePoseEstimation(gpsData, bno055Data);
   ```

3. **Access pose data**:
   ```cpp
   float lat = currentPose.latitude;
   float lon = currentPose.longitude;
   float heading = currentPose.heading;
   float speed = currentPose.speed;
   float confidence = currentPose.confidence;
   ```

## EKF Tuning Guide

All tunable parameters are located at the top of `src/pose_est.cpp`. Adjust these values based on your specific setup and requirements:

### **For Better GPS Performance**
```cpp
const float GPS_POSITION_NOISE = 9.0;     // Decrease for better GPS (was 25.0)
const float GPS_HDOP_THRESHOLD = 5.0;     // Stricter GPS quality (was 20.0)
```
**Use when**: You have a high-quality GPS module with good antenna and clear sky view.

### **For More Responsive Movement**
```cpp
const float POS_PROCESS_NOISE = 0.5;      // More responsive position (was 0.1)
const float VEL_PROCESS_NOISE = 2.0;      // More responsive velocity (was 1.0)
```
**Use when**: Your CanSat moves quickly or changes direction frequently.

### **For Better Heading Tracking**
```cpp
const float HEADING_UPDATE_GAIN = 0.3;    // More responsive heading (was 0.1)
const float HEADING_PROCESS_NOISE = 0.05; // More responsive heading changes (was 0.01)
```
**Use when**: Your magnetometer is stable and you need precise heading control.

### **For Smoother Estimates**
```cpp
const float POS_PROCESS_NOISE = 0.05;     // Less responsive, smoother (was 0.1)
const float VEL_PROCESS_NOISE = 0.5;      // Less responsive, smoother (was 1.0)
```
**Use when**: You want smoother estimates and can tolerate slight delays in response.

### **For Higher Initial Confidence**
```cpp
const float INITIAL_POS_UNCERTAINTY = 50.0;  // More confident start (was 100.0)
const float INITIAL_VEL_UNCERTAINTY = 5.0;   // More confident start (was 10.0)
```
**Use when**: You have very accurate initial position knowledge.

### **For Indoor/Poor GPS Conditions**
```cpp
const float GPS_POSITION_NOISE = 100.0;   // Higher uncertainty for poor GPS
const float GPS_HDOP_THRESHOLD = 50.0;    // Accept lower quality GPS
const float HEADING_UPDATE_GAIN = 0.5;    // Rely more on magnetometer
```

## Parameter Explanations

### **Initial Uncertainty Values**
- **INITIAL_POS_UNCERTAINTY**: How uncertain we are about starting position (m²)
- **INITIAL_VEL_UNCERTAINTY**: How uncertain we are about starting velocity (m²/s²)
- **INITIAL_HEADING_UNCERTAINTY**: How uncertain we are about starting heading (rad²)

### **Process Noise Values**
- **POS_PROCESS_NOISE**: How much position changes between measurements
- **VEL_PROCESS_NOISE**: How much velocity changes between measurements
- **HEADING_PROCESS_NOISE**: How much heading changes between measurements

### **Measurement Noise Values**
- **GPS_POSITION_NOISE**: GPS measurement uncertainty (m²)
- **HEADING_UPDATE_GAIN**: How much to trust magnetometer vs prediction (0-1)

### **Quality Thresholds**
- **GPS_HDOP_THRESHOLD**: Maximum acceptable GPS dilution of precision
- **CONFIDENCE_SCALE**: Scaling factor for confidence calculation

## Troubleshooting

### **GPS Issues**
- If GPS is inaccurate: Increase `GPS_POSITION_NOISE`
- If GPS updates are rejected: Increase `GPS_HDOP_THRESHOLD`
- If position jumps around: Decrease `POS_PROCESS_NOISE`

### **Heading Issues**
- If heading is sluggish: Increase `HEADING_UPDATE_GAIN`
- If heading is noisy: Decrease `HEADING_UPDATE_GAIN`
- If heading drifts: Check magnetometer calibration

### **Motion Issues**
- If movement is too smooth: Increase process noise values
- If movement is too jerky: Decrease process noise values
- If velocity estimates are wrong: Adjust `VEL_PROCESS_NOISE`

## File Structure

```
src/
├── main.cpp           # Main program loop
├── pose_est.h         # EKF pose estimation header
├── pose_est.cpp       # EKF pose estimation implementation
├── gps.h/.cpp         # GPS module interface
├── 9axis.h/.cpp       # BNO055 IMU interface
├── motor.h/.cpp       # Motor control
├── sd_logger.h/.cpp   # SD card logging
└── ultrasonic.h/.cpp  # Ultrasonic sensor
```

## Data Logging

The system automatically logs pose data to `pose_data.csv` with the following columns:
- `timestamp`: System time in milliseconds
- `latitude`: Estimated latitude (degrees)
- `longitude`: Estimated longitude (degrees)
- `heading`: Estimated heading (degrees, 0-360)
- `speed`: Estimated ground speed (m/s)
- `x`: Local X position (meters)
- `y`: Local Y position (meters)
- `confidence`: Estimation confidence (0-1)
- `gps_valid`: GPS data validity flag
- `mag_valid`: Magnetometer data validity flag

## Advanced Tuning

For advanced users, you can modify the EKF matrices directly in the code:
- **P matrix**: Initial covariance (uncertainty)
- **Q matrix**: Process noise covariance
- **R matrix**: Measurement noise covariance

## License

This project is open source. Feel free to modify and use for your CanSat missions!