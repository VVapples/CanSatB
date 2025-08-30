#ifndef SD_LOGGER_H
#define SD_LOGGER_H

#include <Arduino.h>

/**
 * @brief Initializes the SD card and creates a unique directory for the new flight.
 * This function must be called once in your main setup() before any logging.
 * @param csPin The Chip Select (CS) pin for your SD card module.
 * @return true if initialization was successful, false otherwise.
 */
bool setupSdLogger(int csPin);

/**
 * @brief Writes a line of data to a specified file within the flight directory.
 * If the file doesn't exist, it will be created automatically.
 * @param filename The name of the CSV file to write to (e.g., "gps.csv").
 * @param data The complete string of data to write as a new line in the file.
 */
void writeToLog(const String& filename, const String& data);

/**
 * @brief Writes column headers to a CSV file if the file doesn't exist yet.
 * This should be called before the first writeToLog() call for each file.
 * @param filename The name of the CSV file to write headers to (e.g., "gps.csv").
 * @param headers Comma-separated string of column names (e.g., "Time,Latitude,Longitude").
 */
void writeLogHeaders(const String& filename, const String& headers);

#endif // SD_LOGGER_H