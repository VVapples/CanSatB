#ifndef GPS_H
#define GPS_H

#include <Arduino.h>

// Enhanced GPS data structure for Ultimate GPS Breakout v3 (MTK3339 chipset)
struct GpsData {
  // Core Position Data
  double latitude;         // Latitude in degrees (high precision)
  double longitude;        // Longitude in degrees (high precision)
  float altitude;          // Altitude above MSL in meters
  
  // Status Information
  bool hasFix;            // True if GPS has a valid fix
  bool moduleDetected;    // True if GPS module is responding
  uint8_t satelliteCount; // Number of satellites in use
  uint8_t fixQuality;     // 0=Invalid, 1=GPS fix, 2=DGPS fix
  uint8_t fixType;        // 1=No fix, 2=2D fix, 3=3D fix
  
  // Accuracy Metrics (Ultimate GPS v3 provides all DOP values)
  float hdop;             // Horizontal Dilution of Precision
  float vdop;             // Vertical Dilution of Precision  
  float pdop;             // Position Dilution of Precision
  
  // Motion Data (MTK3339 provides accurate speed/course)
  float speed_kmh;        // Speed over ground in km/h
  float speed_knots;      // Speed over ground in knots
  float course;           // Course over ground in degrees
  
  // Time and Date (Ultimate GPS v3 has RTC capability)
  uint8_t hour, minute, second, centisecond;
  uint8_t day, month;
  uint16_t year;
  bool timeValid;         // True if time/date is valid
  
  // Advanced Features (MTK3339 specific)
  uint32_t age;           // Age of GPS data in milliseconds
  uint32_t charsProcessed;// Total characters processed
  uint32_t sentencesWithFix; // Sentences that resulted in a fix
  uint32_t failedChecksum;   // Failed checksum count
  
  // Module Health
  uint32_t lastUpdate;    // Timestamp of last successful update
  float antennaStatus;    // Antenna status (if available)
};

/**
 * @brief Initializes the serial communication with the Ultimate GPS Breakout v3.
 * Sets up communication with MTK3339 chipset and applies optimal configuration.
 * @param txPin The microcontroller pin that sends data TO the GPS RX pin.
 * @param rxPin The microcontroller pin that receives data FROM the GPS TX pin.
 * @return true if GPS initialization was successful, false otherwise.
 */
bool setupGps(int txPin, int rxPin);

/**
 * @brief Configure Ultimate GPS v3 specific settings (MTK3339 commands).
 * @param updateRate Update rate in Hz (1-10 Hz, Ultimate GPS v3 supports up to 10Hz)
 * @param enableRMC Enable RMC sentences (recommended minimum)
 * @param enableGGA Enable GGA sentences (fix data)
 * @param enableGSA Enable GSA sentences (satellite data)
 * @param enableGSV Enable GSV sentences (satellite details)
 * @param enableVTG Enable VTG sentences (course/speed)
 */
void configureUltimateGPS(uint8_t updateRate = 1, bool enableRMC = true, 
                         bool enableGGA = true, bool enableGSA = true, 
                         bool enableGSV = false, bool enableVTG = true);

/**
 * @brief Enable/disable the built-in datalogging feature of Ultimate GPS v3.
 * The MTK3339 can log GPS data to internal flash memory.
 * @param enable True to start logging, false to stop
 * @return true if command was sent successfully
 */
bool setDataLogging(bool enable);

/**
 * @brief Set the antenna type for Ultimate GPS v3.
 * The module can automatically switch between internal and external antenna.
 * @param useExternal True for external antenna, false for internal patch antenna
 */
void setAntennaType(bool useExternal);

/**
 * @brief Reads data from the GPS module and parses it.
 * Enhanced for Ultimate GPS v3 to extract all available data fields.
 * @note This function should be called in every iteration of the main loop().
 * @return Returns true if new, updated location data was received, false otherwise.
 */
bool updateGps();

/**
 * @brief Gets the most recently parsed GPS data.
 * @return A GpsData struct containing the latest information from Ultimate GPS v3.
 */
GpsData getGpsData();

/**
 * @brief Check if the Ultimate GPS v3 module is detected and responding.
 * @return true if module is detected and sending NMEA data, false otherwise.
 */
bool isGpsModuleDetected();

/**
 * @brief Get detailed statistics about GPS module performance.
 * @param totalChars Total characters processed
 * @param validSentences Valid NMEA sentences processed  
 * @param failedChecksums Failed checksum count
 */
void getGpsStatistics(uint32_t &totalChars, uint32_t &validSentences, uint32_t &failedChecksums);

#endif // GPS_H