#include "gps.h"
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
    // Initialize GPS data structure
    memset(&currentGpsData, 0, sizeof(GpsData));
    currentGpsData.moduleDetected = false;
    currentGpsData.hasFix = false;
    currentGpsData.timeValid = false;
    
    // Determine which UART to use based on pins
    int serialNum = 1; // Default to Serial1
    if ((txPin == 1 && rxPin == 3) || (txPin == 3 && rxPin == 1)) {
        serialNum = 0; // Serial0 (USB)
    } else if ((txPin == 17 && rxPin == 16) || (txPin == 16 && rxPin == 17)) {
        serialNum = 2; // Serial2
    }
    
    // Initialize hardware serial for Ultimate GPS v3
    gpsSerial = new HardwareSerial(serialNum);
    if (gpsSerial == nullptr) {
        return false;
    }
    
    gpsSerial->begin(GPS_BAUD_RATE, SERIAL_8N1, rxPin, txPin);
    moduleInitTime = millis();
    
    // Wait for GPS module to initialize
    delay(1000);
    
    // Configure Ultimate GPS v3 for optimal performance
    configureUltimateGPS(1, true, true, true, false, true);
    
    return true;
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
    if (gpsSerial == nullptr) return false;
    
    bool newData = false;
    
    // Process all available GPS data
    while (gpsSerial->available() > 0) {
        char c = gpsSerial->read();
        if (gps.encode(c)) {
            lastDataReceived = millis();
            moduleResponding = true;
            newData = true;
        }
    }
    
    // Check module timeout
    if (millis() - lastDataReceived > MODULE_TIMEOUT) {
        moduleResponding = false;
    }
    
    currentGpsData.moduleDetected = moduleResponding;
    
    // Update GPS data if location was updated
    if (gps.location.isUpdated() || gps.time.isUpdated() || gps.date.isUpdated()) {
        currentGpsData.lastUpdate = millis();
        
        // Position data (high precision for Ultimate GPS v3)
        if (gps.location.isValid()) {
            currentGpsData.hasFix = true;
            currentGpsData.latitude = gps.location.lat();
            currentGpsData.longitude = gps.location.lng();
        } else {
            currentGpsData.hasFix = false;
        }
        
        // Altitude
        if (gps.altitude.isValid()) {
            currentGpsData.altitude = gps.altitude.meters();
        }
        
        // Satellite information
        if (gps.satellites.isValid()) {
            currentGpsData.satelliteCount = gps.satellites.value();
        }
        
        // Dilution of Precision (Ultimate GPS v3 provides all DOP values)
        if (gps.hdop.isValid()) {
            currentGpsData.hdop = gps.hdop.hdop();
        }
        
        // Speed data (Ultimate GPS v3 provides accurate speed)
        if (gps.speed.isValid()) {
            currentGpsData.speed_kmh = gps.speed.kmph();
            currentGpsData.speed_knots = gps.speed.knots();
        }
        
        // Course over ground
        if (gps.course.isValid()) {
            currentGpsData.course = gps.course.deg();
        }
        
        // Time and date (Ultimate GPS v3 has RTC backup)
        if (gps.time.isValid() && gps.date.isValid()) {
            currentGpsData.timeValid = true;
            currentGpsData.hour = gps.time.hour();
            currentGpsData.minute = gps.time.minute();
            currentGpsData.second = gps.time.second();
            currentGpsData.centisecond = gps.time.centisecond();
            currentGpsData.day = gps.date.day();
            currentGpsData.month = gps.date.month();
            currentGpsData.year = gps.date.year();
        } else {
            currentGpsData.timeValid = false;
        }
        
        // Fix quality and type inference
        if (currentGpsData.hasFix) {
            currentGpsData.fixQuality = 1; // GPS fix
            currentGpsData.fixType = (gps.altitude.isValid()) ? 3 : 2; // 3D or 2D fix
        } else {
            currentGpsData.fixQuality = 0; // Invalid
            currentGpsData.fixType = 1;    // No fix
        }
        
        // GPS statistics (Ultimate GPS v3 diagnostics)
        currentGpsData.age = gps.location.age();
        currentGpsData.charsProcessed = gps.charsProcessed();
        currentGpsData.sentencesWithFix = gps.sentencesWithFix();
        currentGpsData.failedChecksum = gps.failedChecksum();
        
        return true;
    }
    
    return false;
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