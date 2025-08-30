#include "sd_logger.h"
#include "SD.h"
#include "SPI.h"

// A private (static) variable to store the path of the current flight directory
static String flightDir;
// A flag to ensure we don't try to write data if the SD card failed to start
static bool sdInitialized = false;

bool setupSdLogger(int csPin) {
  if (!SD.begin(csPin)) {
    sdInitialized = false;
    return false;
  }

  // Find the next available flight directory number
  for (int i = 0; i < 1000; i++) {
    char dirName[15];
    // Formats the name to "/FLIGHT_000", "/FLIGHT_001", etc.
    sprintf(dirName, "/FLIGHT_%03d", i);

    if (!SD.exists(dirName)) {
      // Create the new directory
      if (SD.mkdir(dirName)) {
        flightDir = String(dirName);
        sdInitialized = true;
        return true;
      } else {
        sdInitialized = false;
        return false;
      }
    }
  }

  // If all 1000 folders are full (highly unlikely)
  sdInitialized = false;
  return false;
}

void writeToLog(const String& filename, const String& data) {
  // Guard clause: Do not attempt to write if the SD card isn't ready.
  if (!sdInitialized) {
    Serial.println("DEBUG: SD not initialized, cannot write to " + filename);
    return;
  }

  // Combine the flight directory and filename to get the full path
  String filePath = flightDir + "/" + filename;
  
  // Debug: Check if file exists before opening
  bool fileExisted = SD.exists(filePath);
  Serial.print("DEBUG: File ");
  Serial.print(filePath);
  Serial.print(fileExisted ? " exists" : " does not exist");

  // Open the file in append mode (FILE_APPEND). This creates the file if it doesn't
  // exist and moves the cursor to the end automatically.
  File logFile = SD.open(filePath, FILE_APPEND);

  if (logFile) {
    // Debug: Check file size before writing
    size_t sizeBefore = logFile.size();
    Serial.print(" - Size before: ");
    Serial.print(sizeBefore);
    
    // If the file opened successfully, write the data.
    logFile.println(data);
    
    // Debug: Check file size after writing
    size_t sizeAfter = logFile.size();
    Serial.print(" bytes, after: ");
    Serial.print(sizeAfter);
    Serial.println(" bytes");
    
    // Close the file to save the data and prevent corruption.
    logFile.close();
    
    Serial.println("DEBUG: Successfully wrote: " + data);
  } else {
    Serial.println("DEBUG: Failed to open file " + filePath);
  }
}

void writeLogHeaders(const String& filename, const String& headers) {
  // Guard clause: Do not attempt to write if the SD card isn't ready.
  if (!sdInitialized) {
    Serial.println("DEBUG: SD not initialized, cannot write headers to " + filename);
    return;
  }

  // Combine the flight directory and filename to get the full path
  String filePath = flightDir + "/" + filename;

  // Check if the file already exists
  if (!SD.exists(filePath)) {
    Serial.println("DEBUG: Writing headers to new file: " + filePath);
    // File doesn't exist, so we can write headers
    File logFile = SD.open(filePath, FILE_WRITE);
    
    if (logFile) {
      // Write the headers as the first line
      logFile.println(headers);
      // Close the file to save the headers
      logFile.close();
      Serial.println("DEBUG: Headers written successfully: " + headers);
    } else {
      Serial.println("DEBUG: Failed to open file for headers: " + filePath);
    }
  } else {
    Serial.println("DEBUG: File already exists, skipping headers: " + filePath);
  }
  // If file already exists, do nothing (headers already written)
}