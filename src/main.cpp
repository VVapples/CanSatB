#include <Arduino.h>
#include "sd_logger.h"
#include <SD.h> // Add this for direct SD access


//pins SETLATER


// put function declarations here:
int myFunction(int, int);

void setup() {
  //Serial w/ pc
  Serial.begin(115200);

  // Initialize SD card (use CS pin 10 as example, change if needed)
  const int csPin = 10;
  if (!setupSdLogger(csPin)) {
    Serial.println("SD card initialization failed!");
    while (1); // Halt if SD card fails
  }
  Serial.println("SD card initialized.");

  // Write test string using writeToLog
  const char* testFilename = "test.txt";
  const char* testData = "Hello SD Card!";
  writeToLog(testFilename, testData);
  Serial.print("Wrote to SD: ");
  Serial.println(testData);

  // Read back the string using SD library directly
  String filePath = "/FLIGHT_000/" + String(testFilename); // Assumes first flight dir
  File testFile = SD.open(filePath, FILE_READ);
  if (testFile) {
    Serial.print("Read from SD: ");
    while (testFile.available()) {
      Serial.write(testFile.read());
    }
    Serial.println();
    testFile.close();
  } else {
    Serial.println("Failed to read from SD card.");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
