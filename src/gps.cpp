#include "gps.h"
#include "sd_logger.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>

// Serial connection to Ultimate GPS Breakout v3 (MTK3339)
static HardwareSerial* gpsSerial = nullptr;

// TinyGPS++ parser optimized for Ultimate GPS v3
static TinyGPSPlus gps;

// Current GPS data structure
static GpsData currentGpsData;

// Ultimate GPS v3 specific constants
static const uint32_t GPS_BAUD_RATE = 9600;  // MTK3339 default baud rate
static const uint32_t MODULE_TIMEOUT = 10000; // 10 second timeout for module detection

// Module detection and health monitoring
static uint32_t lastDataReceived = 0;
static uint32_t moduleInitTime = 0;
static bool moduleResponding = false;

// MTK3339 Command Templates for Ultimate GPS v3
static const char* MTK_SET_NMEA_UPDATE_RATE = "$PMTK220,%d*"; // Update rate command
static const char* MTK_SET_NMEA_OUTPUT = "$PMTK314,0,1,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*"; // Output format
static const char* MTK_API_SET_FIX_CTL = "$PMTK300,%d,0,0,0,0*"; // Fix control
static const char* MTK_SET_DATUM = "$PMTK330,0*"; // Set datum to WGS84
static const char* MTK_ENABLE_SBAS = "$PMTK313,1*"; // Enable SBAS
static const char* MTK_ENABLE_WAAS = "$PMTK301,2*"; // Enable WAAS

// Calculate MTK command checksum
uint8_t calculateMTKChecksum(const char* command) {
    uint8_t checksum = 0;
    // Skip the '$' and calculate until '*'
    for (int i = 1; command[i] != '*' && command[i] != '\0'; i++) {
        checksum ^= command[i];
    }
    return checksum;
}

// Send MTK command to Ultimate GPS v3
void sendMTKCommand(const char* command) {
    if (gpsSerial == nullptr) return;
    
    char fullCommand[100];
    strcpy(fullCommand, command);
    
    // Find the '*' and add checksum
    char* asterisk = strchr(fullCommand, '*');
    if (asterisk != nullptr) {
        uint8_t checksum = calculateMTKChecksum(fullCommand);
        sprintf(asterisk, "*%02X\r\n", checksum);
    } else {
        strcat(fullCommand, "\r\n");
    }
    
    gpsSerial->print(fullCommand);
    delay(100); // Give GPS time to process command
}
bool setupGps(int txPin, int rxPin) {
    writeToLog("debug.csv", String(millis()) + ",GPS,SETUP_START,0,GPS Setup Starting - TX:" + String(txPin) + " RX:" + String(rxPin));
    
    try {
        // Initialize GPS data structure
        memset(&currentGpsData, 0, sizeof(GpsData));
        currentGpsData.latitude = 0.0;
        currentGpsData.longitude = 0.0;
        currentGpsData.altitude = 0.0;
        currentGpsData.satelliteCount = 0;
        currentGpsData.hdop = 999.99;
        currentGpsData.hasFix = false;
        
        writeToLog("debug.csv", String(millis()) + ",GPS,DATA_INIT,0,GPS data structure initialized");
        
        // Create GPS serial connection
        gpsSerial = &Serial1;
        writeToLog("debug.csv", String(millis()) + ",GPS,SERIAL_SELECT,0,Using Serial1 for GPS");
        
        // Initialize with pins and baud rate
        gpsSerial->begin(9600, SERIAL_8N1, rxPin, txPin);
        writeToLog("debug.csv", String(millis()) + ",GPS,SERIAL_INIT,0,Serial initialized at 9600 baud");
        
        // Clear any existing data
        delay(100);
        while (gpsSerial->available()) {
            gpsSerial->read();
        }
        writeToLog("debug.csv", String(millis()) + ",GPS,BUFFER_CLEAR,0,Serial buffer cleared");
        
        // Test 1: Basic communication test
        writeToLog("debug.csv", String(millis()) + ",GPS,TEST1_START,0,Starting basic communication test");
        
        unsigned long testStart = millis();
        int bytesReceived = 0;
        char testBuffer[100];
        int bufferIndex = 0;
        
        // Listen for 3 seconds for ANY data
        while (millis() - testStart < 3000 && bufferIndex < 99) {
            if (gpsSerial->available()) {
                char c = gpsSerial->read();
                testBuffer[bufferIndex++] = c;
                bytesReceived++;
            }
            delay(1);
        }
        testBuffer[bufferIndex] = '\0';
        
        writeToLog("debug.csv", String(millis()) + ",GPS,TEST1_RESULT,0,Received " + String(bytesReceived) + " bytes in 3 seconds");
        
        if (bytesReceived == 0) {
            writeToLog("debug.csv", String(millis()) + ",GPS,TEST1_FAIL,0,No data received - checking wiring and power");
            return false;
        }
        
        // Log first few characters received
        String firstChars = "";
        for (int i = 0; i < min(20, bufferIndex); i++) {
            if (testBuffer[i] >= 32 && testBuffer[i] <= 126) {
                firstChars += testBuffer[i];
            } else {
                firstChars += "[" + String((int)testBuffer[i]) + "]";
            }
        }
        writeToLog("debug.csv", String(millis()) + ",GPS,TEST1_DATA,0,First chars: " + firstChars);
        
        // Test 2: Look for NMEA sentences
        writeToLog("debug.csv", String(millis()) + ",GPS,TEST2_START,0,Looking for NMEA sentences");
        
        testStart = millis();
        int nmeaLines = 0;
        String currentLine = "";
        
        while (millis() - testStart < 5000 && nmeaLines < 3) {
            if (gpsSerial->available()) {
                char c = gpsSerial->read();
                
                if (c == '\n' || c == '\r') {
                    if (currentLine.length() > 0) {
                        if (currentLine.startsWith("$")) {
                            nmeaLines++;
                            writeToLog("debug.csv", String(millis()) + ",GPS,NMEA_FOUND,0,NMEA: " + currentLine.substring(0, min(50, (int)currentLine.length())));
                        }
                        currentLine = "";
                    }
                } else {
                    currentLine += c;
                }
            }
            delay(1);
        }
        
        if (nmeaLines == 0) {
            writeToLog("debug.csv", String(millis()) + ",GPS,TEST2_FAIL,0,No NMEA sentences found - GPS module not responding properly");
            return false;
        }
        
        writeToLog("debug.csv", String(millis()) + ",GPS,TEST2_SUCCESS,0,Found " + String(nmeaLines) + " NMEA sentences");
        
        // Test 3: Test TinyGPS++ parsing
        writeToLog("debug.csv", String(millis()) + ",GPS,TEST3_START,0,Testing TinyGPS++ parsing");
        
        testStart = millis();
        int parsedSentences = 0;
        int totalChars = 0;
        
        while (millis() - testStart < 5000) {
            if (gpsSerial->available()) {
                char c = gpsSerial->read();
                totalChars++;
                
                if (gps.encode(c)) {
                    parsedSentences++;
                }
            }
            delay(1);
        }
        
        writeToLog("debug.csv", String(millis()) + ",GPS,TEST3_RESULT,0,Parsed " + String(parsedSentences) + " sentences from " + String(totalChars) + " chars");
        
        if (parsedSentences == 0) {
            writeToLog("debug.csv", String(millis()) + ",GPS,TEST3_FAIL,0,TinyGPS++ could not parse any sentences");
            return false;
        }
        
        // Initialize GPS data CSV
        String headers = "timestamp,valid,latitude,longitude,altitude,satellites,hdop,vdop,pdop,speed_kmh,speed_knots,course,fix_quality,fix_type,data_age,chars_processed,sentences_parsed,checksum_errors,hour,minute,second,day,month,year,time_valid";
        writeLogHeaders("gps_data.csv", headers);
        writeToLog("debug.csv", String(millis()) + ",GPS,CSV_INIT,0,GPS data CSV initialized");
        
        writeToLog("debug.csv", String(millis()) + ",GPS,SETUP_SUCCESS,0,GPS module detected and initialized successfully");
        return true;
        
    } catch (...) {
        writeToLog("debug.csv", String(millis()) + ",GPS,SETUP_EXCEPTION,0,Exception caught during GPS setup");
        return false;
    }
}
void configureUltimateGPS(uint8_t updateRate, bool enableRMC, bool enableGGA, 
                         bool enableGSA, bool enableGSV, bool enableVTG) {
    if (gpsSerial == nullptr) return;
    
    // Set update rate (1-10 Hz for Ultimate GPS v3)
    if (updateRate >= 1 && updateRate <= 10) {
        char rateCmd[50];
        uint16_t period = 1000 / updateRate;
        sprintf(rateCmd, "$PMTK220,%d*", period);
        sendMTKCommand(rateCmd);
    }
    
    // Configure NMEA sentence output
    char outputCmd[100];
    sprintf(outputCmd, "$PMTK314,0,%d,0,%d,%d,%d,0,0,0,0,0,0,0,0,0,0,0,%d,0*",
            enableRMC ? 1 : 0,  // RMC
            enableGGA ? 1 : 0,  // GGA  
            enableGSA ? 1 : 0,  // GSA
            enableGSV ? 1 : 0,  // GSV
            enableVTG ? 1 : 0); // VTG
    sendMTKCommand(outputCmd);
    
    // Enable SBAS (WAAS/EGNOS) for better accuracy
    sendMTKCommand("$PMTK313,1*");
    sendMTKCommand("$PMTK301,2*");
    
    // Set datum to WGS84
    sendMTKCommand("$PMTK330,0*");
    
    // Hot start for faster fix (Ultimate GPS v3 feature)
    sendMTKCommand("$PMTK101*");
}

bool setDataLogging(bool enable) {
    if (gpsSerial == nullptr) return false;
    
    if (enable) {
        // Start logging (Ultimate GPS v3 internal flash logging)
        sendMTKCommand("$PMTK185,0*");
        delay(100);
        sendMTKCommand("$PMTK185,1*");
    } else {
        // Stop logging
        sendMTKCommand("$PMTK185,0*");
    }
    
    return true;
}

void setAntennaType(bool useExternal) {
    if (gpsSerial == nullptr) return;
    
    if (useExternal) {
        // Configure for external active antenna
        sendMTKCommand("$PGCMD,33,1*");
    } else {
        // Configure for internal patch antenna  
        sendMTKCommand("$PGCMD,33,0*");
    }
}
bool updateGps() {
    if (!gpsSerial) {
        return false;
    }
    
    static unsigned long lastUpdate = 0;
    static unsigned long lastLog = 0;
    static unsigned long lastDebugLog = 0;
    
    // Process available GPS data
    int charsProcessed = 0;
    bool newDataReceived = false;
    
    while (gpsSerial->available() && charsProcessed < 200) {
        char c = gpsSerial->read();
        charsProcessed++;
        
        if (gps.encode(c)) {
            // New sentence parsed
            newDataReceived = true;
            lastDataReceived = millis();
            moduleResponding = true;
        }
    }
    
    // Check module timeout
    if (millis() - lastDataReceived > MODULE_TIMEOUT) {
        moduleResponding = false;
    }
    
    // Update GPS data every second
    if (millis() - lastUpdate >= 1000) {
        lastUpdate = millis();
        
        // Update position data
        if (gps.location.isValid()) {
            currentGpsData.latitude = gps.location.lat();
            currentGpsData.longitude = gps.location.lng();
            currentGpsData.hasFix = true;
            currentGpsData.age = gps.location.age();
        } else {
            currentGpsData.hasFix = false;
        }
        
        // Update other data
        currentGpsData.satelliteCount = gps.satellites.isValid() ? gps.satellites.value() : 0;
        currentGpsData.altitude = gps.altitude.isValid() ? gps.altitude.meters() : 0.0;
        currentGpsData.hdop = gps.hdop.isValid() ? gps.hdop.hdop() : 999.99;
        currentGpsData.speed_kmh = gps.speed.isValid() ? gps.speed.kmph() : 0.0;
        currentGpsData.speed_knots = gps.speed.isValid() ? gps.speed.knots() : 0.0;
        currentGpsData.course = gps.course.isValid() ? gps.course.deg() : 0.0;
        
        // Time data
        if (gps.time.isValid()) {
            currentGpsData.hour = gps.time.hour();
            currentGpsData.minute = gps.time.minute();
            currentGpsData.second = gps.time.second();
            currentGpsData.timeValid = true;
        } else {
            currentGpsData.timeValid = false;
        }
        
        // Date data
        if (gps.date.isValid()) {
            currentGpsData.day = gps.date.day();
            currentGpsData.month = gps.date.month();
            currentGpsData.year = gps.date.year();
        }
        
        // Statistics
        currentGpsData.charsProcessed = gps.charsProcessed();
        currentGpsData.sentencesWithFix = gps.sentencesWithFix();
        currentGpsData.failedChecksum = gps.failedChecksum();
        currentGpsData.moduleDetected = moduleResponding;
        
        // Fix quality and type
        if (currentGpsData.hasFix) {
            currentGpsData.fixQuality = 1;
            currentGpsData.fixType = (gps.altitude.isValid()) ? 3 : 2;
        } else {
            currentGpsData.fixQuality = 0;
            currentGpsData.fixType = 1;
        }
    }
    
    // Log GPS data every 5 seconds
    if (millis() - lastLog >= 5000) {
        lastLog = millis();
        
        String gpsDataLine = String(millis()) + "," +
                           String(currentGpsData.hasFix ? "1" : "0") + "," +
                           String(currentGpsData.latitude, 6) + "," +
                           String(currentGpsData.longitude, 6) + "," +
                           String(currentGpsData.altitude, 2) + "," +
                           String(currentGpsData.satelliteCount) + "," +
                           String(currentGpsData.hdop, 2) + "," +
                           String(currentGpsData.vdop, 2) + "," +
                           String(currentGpsData.pdop, 2) + "," +
                           String(currentGpsData.speed_kmh, 2) + "," +
                           String(currentGpsData.speed_knots, 2) + "," +
                           String(currentGpsData.course, 2) + "," +
                           String(currentGpsData.fixQuality) + "," +
                           String(currentGpsData.fixType) + "," +
                           String(currentGpsData.age) + "," +
                           String(currentGpsData.charsProcessed) + "," +
                           String(currentGpsData.sentencesWithFix) + "," +
                           String(currentGpsData.failedChecksum) + "," +
                           String(currentGpsData.hour) + "," +
                           String(currentGpsData.minute) + "," +
                           String(currentGpsData.second) + "," +
                           String(currentGpsData.day) + "," +
                           String(currentGpsData.month) + "," +
                           String(currentGpsData.year) + "," +
                           String(currentGpsData.timeValid ? "1" : "0");
        
        writeToLog("gps_data.csv", gpsDataLine);
    }
    
    // Debug status every 15 seconds
    if (millis() - lastDebugLog >= 15000) {
        lastDebugLog = millis();
        writeToLog("debug.csv", String(millis()) + ",GPS,STATUS,0,Module:" + String(moduleResponding ? "OK" : "TIMEOUT") + 
                  " Sats:" + String(currentGpsData.satelliteCount) + 
                  " Fix:" + String(currentGpsData.hasFix ? "YES" : "NO") + 
                  " HDOP:" + String(currentGpsData.hdop, 1) + 
                  " Chars:" + String(currentGpsData.charsProcessed) + 
                  " CharsNow:" + String(charsProcessed));
    }
    
    return newDataReceived;
}

GpsData getGpsData() {
    return currentGpsData;
}

bool isGpsModuleDetected() {
    return moduleResponding && (millis() - lastDataReceived < MODULE_TIMEOUT);
}

void getGpsStatistics(uint32_t &totalChars, uint32_t &validSentences, uint32_t &failedChecksums) {
    totalChars = gps.charsProcessed();
    validSentences = gps.sentencesWithFix();
    failedChecksums = gps.failedChecksum();
}