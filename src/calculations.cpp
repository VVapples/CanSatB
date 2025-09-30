#include "calculations.h"
#include <math.h>

// Earth's radius in meters
#define EARTH_RADIUS_M 6371000.0

// Heading offset to correct for BNO055 vs robot orientation difference (in degrees)
// Positive values rotate clockwise, negative values rotate counterclockwise
// Default: 90 degrees (adjust this value to match your robot's mounting)
double HEADING_OFFSET = 90.0;

/**
 * Convert degrees to radians
 */
double toRadians(double degrees) {
    return degrees * M_PI / 180.0;
}

/**
 * Convert radians to degrees
 */
double toDegrees(double radians) {
    return radians * 180.0 / M_PI;
}

/**
 * Calculate the distance between two GPS coordinates using the Haversine formula
 * 
 * The Haversine formula calculates the great-circle distance between two points
 * on a sphere given their latitude and longitude coordinates.
 * 
 * Formula:
 * a = sin²(Δφ/2) + cos φ1 ⋅ cos φ2 ⋅ sin²(Δλ/2)
 * c = 2 ⋅ atan2( √a, √(1−a) )
 * d = R ⋅ c
 * 
 * where φ is latitude, λ is longitude, R is earth's radius
 */
double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
    // Convert coordinates to radians
    double lat1_rad = toRadians(lat1);
    double lon1_rad = toRadians(lon1);
    double lat2_rad = toRadians(lat2);
    double lon2_rad = toRadians(lon2);
    
    // Calculate differences
    double delta_lat = lat2_rad - lat1_rad;
    double delta_lon = lon2_rad - lon1_rad;
    
    // Haversine formula
    double a = sin(delta_lat / 2.0) * sin(delta_lat / 2.0) +
               cos(lat1_rad) * cos(lat2_rad) *
               sin(delta_lon / 2.0) * sin(delta_lon / 2.0);
    
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    
    // Distance in meters
    double distance = EARTH_RADIUS_M * c;
    
    return distance;
}

/**
 * Calculate magnetic heading using magnetometer X and Y components
 * @param magX - Magnetometer X component (microteslas)
 * @param magY - Magnetometer Y component (microteslas)
 * @return heading in degrees (-180 to +180), where 0° = North, 90° = East, -90° = West, ±180° = South
 */
double calculateMagneticHeading(double magX, double magY) {
  // Calculate heading using atan2 (handles all quadrants correctly)
  double heading = atan2(magY, magX) * 180.0 / M_PI;
  
  // atan2 gives us the angle from the positive X-axis
  // For compass heading, we need angle from North (Y-axis)
  // So we subtract 90 degrees to rotate the reference
  heading = heading - 90.0;
  
  // Apply robot orientation offset to correct for BNO055 mounting
  heading = heading + HEADING_OFFSET;
  
  // Normalize to -180 to +180 range
  if (heading > 180.0) {
    heading -= 360.0;
  } else if (heading < -180.0) {
    heading += 360.0;
  }
  
  return heading;
}

/**
 * Calculate the bearing (direction) from one GPS coordinate to another
 * 
 * Formula:
 * θ = atan2( sin Δlong * cos lat2, cos lat1 * sin lat2 − sin lat1 * cos lat2 * cos Δlong )
 * 
 * @return Bearing in degrees (0-360, where 0 is North, 90 is East, etc.)
 */
double calculateBearing(double lat1, double lon1, double lat2, double lon2) {
    // Convert coordinates to radians
    double lat1_rad = toRadians(lat1);
    double lon1_rad = toRadians(lon1);
    double lat2_rad = toRadians(lat2);
    double lon2_rad = toRadians(lon2);
    
    // Calculate difference in longitude
    double delta_lon = lon2_rad - lon1_rad;
    
    // Calculate bearing
    double y = sin(delta_lon) * cos(lat2_rad);
    double x = cos(lat1_rad) * sin(lat2_rad) - 
               sin(lat1_rad) * cos(lat2_rad) * cos(delta_lon);
    
    double bearing_rad = atan2(y, x);
    
    // Convert to degrees and normalize to 0-360
    double bearing_deg = toDegrees(bearing_rad);
    bearing_deg = fmod(bearing_deg + 360.0, 360.0);
    
    return bearing_deg;
}

/**
 * Set the heading offset to correct for BNO055 vs robot orientation difference
 * @param offset_degrees - Offset in degrees (positive = clockwise rotation)
 */
void setHeadingOffset(double offset_degrees) {
  HEADING_OFFSET = offset_degrees;
}

/**
 * Get the current heading offset value
 * @return Current heading offset in degrees
 */
double getHeadingOffset() {
  return HEADING_OFFSET;
}