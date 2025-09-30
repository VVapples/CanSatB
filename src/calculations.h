#ifndef CALCULATIONS_H
#define CALCULATIONS_H

/**
 * GPS and Navigation Calculations Library
 * 
 * Functions for calculating distances, bearings, and other navigation-related calculations
 */

/**
 * Calculate the distance between two GPS coordinates using the Haversine formula
 * 
 * @param lat1 Latitude of first point in decimal degrees
 * @param lon1 Longitude of first point in decimal degrees
 * @param lat2 Latitude of second point in decimal degrees
 * @param lon2 Longitude of second point in decimal degrees
 * @return Distance in meters
 */
double calculateDistance(double lat1, double lon1, double lat2, double lon2);

/**
 * Calculate the bearing (direction) from one GPS coordinate to another
 * 
 * @param lat1 Latitude of starting point in decimal degrees
 * @param lon1 Longitude of starting point in decimal degrees
 * @param lat2 Latitude of destination point in decimal degrees
 * @param lon2 Longitude of destination point in decimal degrees
 * @return Bearing in degrees (0-360, where 0 is North)
 */
double calculateBearing(double lat1, double lon1, double lat2, double lon2);

/**
 * Calculate magnetic heading using magnetometer X and Y components
 * 
 * @param magX Magnetometer X component (microteslas)
 * @param magY Magnetometer Y component (microteslas)
 * @return Heading in degrees (-180 to +180), where 0° = North, 90° = East, -90° = West, ±180° = South
 */
double calculateMagneticHeading(double magX, double magY);

/**
 * Set the heading offset to correct for BNO055 vs robot orientation difference
 * 
 * @param offset_degrees Offset in degrees (positive = clockwise rotation)
 */
void setHeadingOffset(double offset_degrees);

/**
 * Get the current heading offset value
 * 
 * @return Current heading offset in degrees
 */
double getHeadingOffset();

#endif // CALCULATIONS_H