#ifndef CALCULATIONS_H
#define CALCULATIONS_H

#include "pose_est.h"  // For Pose structure

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
 * Calculate how far off the current heading is from pointing toward the target
 * 
 * @param currentPose Current pose containing latitude, longitude, and heading
 * @param targetPose Target pose containing latitude and longitude (heading ignored)
 * @return Heading error in degrees (-180 to +180)
 *         0 = heading directly toward target
 *         Positive = need to turn right (clockwise)
 *         Negative = need to turn left (counter-clockwise)
 */
double calculateHeadingError(const Pose& currentPose, const Pose& targetPose);

#endif // CALCULATIONS_H