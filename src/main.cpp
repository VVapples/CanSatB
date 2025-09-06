#include <Arduino.h>

// M5Stack Unit-Cam Pin Definitions
#define UNITCAM_RX 17    // ESP32 GPIO17 -> Unit-Cam G3 (TX)
#define UNITCAM_TX 16    // ESP32 GPIO16 -> Unit-Cam G1 (RX)

// Use Hardware Serial 2 for Unit-Cam communication (ESP32 has multiple hardware serials)
HardwareSerial unitCamSerial(2);

// Test parameters
#define CAMERA_BAUD_RATE 9600
#define STREAM_INTERVAL 1000   // Stream every 1 second (1 FPS)
#define RESPONSE_TIMEOUT 2000  // Timeout for camera responses
#define MAX_IMAGE_SIZE 8192    // Maximum expected image size in bytes
#define STREAM_MODE true       // Set to true for streaming, false for testing

// Function declarations
void setupUnitCam();
void streamCameraLoop();
void captureAndStreamImage();
void testUnitCamCommunication();
void sendCameraCommand(String command);
void readCameraResponse();
void testBasicCommands();
void testCameraInfo();
void testImageCapture();
void printHexData(uint8_t* data, int length);
void sendImageOverSerial(uint8_t* imageData, int imageSize);

void setup() {
  // Initialize USB Serial for PC communication at higher baud rate for streaming
  Serial.begin(115200);
  
  // Wait for serial to initialize
  while (!Serial && millis() < 5000) {
    delay(100);
  }
  
  if (STREAM_MODE) {
    Serial.println("========================================");
    Serial.println("M5Stack Unit-Cam STREAMING Mode");
    Serial.println("========================================");
    Serial.println("Starting camera stream at 1 FPS...");
    Serial.println("Use Python script to view stream on PC");
  } else {
    Serial.println("========================================");
    Serial.println("M5Stack Unit-Cam Test Program");
    Serial.println("========================================");
    Serial.println("ESP32 <-> Unit-Cam Pin Configuration:");
    Serial.println("  GPIO16 -> Unit-Cam G1 (RX)");
    Serial.println("  GPIO17 -> Unit-Cam G3 (TX)");
  }
  Serial.println("========================================");
  
  // Initialize Unit-Cam
  setupUnitCam();
  
  if (STREAM_MODE) {
    Serial.println("STREAM_START"); // Marker for Python script
  } else {
    Serial.println("Setup complete. Starting camera tests...");
  }
  delay(2000);
}

void loop() {
  if (STREAM_MODE) {
    // Streaming mode - capture and send images continuously
    streamCameraLoop();
  } else {
    // Test mode - run diagnostic tests
    Serial.println("\n=== Starting Unit-Cam Test Cycle ===");
    
    // Test basic communication
    testUnitCamCommunication();
    delay(1000);
    
    // Test camera information
    testCameraInfo();
    delay(1000);
    
    // Test basic commands
    testBasicCommands();
    delay(1000);
    
    // Test image capture
    testImageCapture();
    
    Serial.println("=== Test Cycle Complete ===");
    Serial.println("Waiting 5 seconds before next cycle...\n");
    delay(5000);
  }
}

// Setup Unit-Cam communication
void setupUnitCam() {
  Serial.println("Initializing Unit-Cam communication...");
  
  // Initialize Serial2 for Unit-Cam communication
  unitCamSerial.begin(CAMERA_BAUD_RATE, SERIAL_8N1, UNITCAM_RX, UNITCAM_TX);
  
  Serial.println("Unit-Cam serial initialized at " + String(CAMERA_BAUD_RATE) + " baud");
  
  // Clear any existing data in the buffer
  while (unitCamSerial.available()) {
    unitCamSerial.read();
  }
  
  Serial.println("Unit-Cam initialization complete.");
}

// Test basic communication with Unit-Cam
void testUnitCamCommunication() {
  Serial.println("\n--- Testing Unit-Cam Communication ---");
  
  // Send a simple command to test communication
  Serial.println("Sending test command...");
  unitCamSerial.println("AT");
  
  // Wait for response
  delay(500);
  
  if (unitCamSerial.available()) {
    Serial.println("✓ Unit-Cam is responding!");
    readCameraResponse();
  } else {
    Serial.println("✗ No response from Unit-Cam");
    Serial.println("  Check connections and power supply");
  }
}

// Send command to camera and print what was sent
void sendCameraCommand(String command) {
  Serial.println("→ Sending: " + command);
  unitCamSerial.println(command);
  unitCamSerial.flush(); // Wait for transmission to complete
}

// Read and display camera response
void readCameraResponse() {
  String response = "";
  unsigned long startTime = millis();
  
  Serial.print("← Response: ");
  
  while (millis() - startTime < RESPONSE_TIMEOUT) {
    if (unitCamSerial.available()) {
      char c = unitCamSerial.read();
      response += c;
      Serial.print(c);
      
      // Reset timeout if we're still receiving data
      startTime = millis();
    }
    delay(1);
  }
  
  if (response.length() == 0) {
    Serial.println("(No response)");
  } else {
    Serial.println();
    Serial.println("  Response length: " + String(response.length()) + " characters");
  }
}

// Test camera information commands
void testCameraInfo() {
  Serial.println("\n--- Testing Camera Information ---");
  
  // Test various AT commands that might work with Unit-Cam
  String commands[] = {
    "AT",           // Basic AT command
    "AT+VERSION",   // Version information
    "AT+INFO",      // Camera information
    "AT+STATUS",    // Camera status
    "AT+ID"         // Camera ID
  };
  
  for (int i = 0; i < 5; i++) {
    sendCameraCommand(commands[i]);
    delay(500);
    readCameraResponse();
    delay(1000);
  }
}

// Test basic camera commands
void testBasicCommands() {
  Serial.println("\n--- Testing Basic Camera Commands ---");
  
  // Test basic camera control commands
  String commands[] = {
    "AT+RESET",     // Reset camera
    "AT+INIT",      // Initialize camera
    "AT+QUALITY=1", // Set image quality
    "AT+SIZE=1",    // Set image size
    "AT+BAUD=9600"  // Set baud rate
  };
  
  for (int i = 0; i < 5; i++) {
    sendCameraCommand(commands[i]);
    delay(1000);
    readCameraResponse();
    delay(1000);
  }
}

// Test image capture
void testImageCapture() {
  Serial.println("\n--- Testing Image Capture ---");
  
  Serial.println("Attempting to capture image...");
  sendCameraCommand("AT+CAPTURE");
  delay(2000); // Give more time for image capture
  
  Serial.println("Reading capture response...");
  readCameraResponse();
  
  // Try alternative capture commands
  delay(1000);
  sendCameraCommand("AT+PHOTO");
  delay(2000);
  readCameraResponse();
  
  // Check for any binary data (image data)
  delay(1000);
  if (unitCamSerial.available()) {
    Serial.println("Binary data available, reading first 100 bytes...");
    uint8_t buffer[100];
    int bytesRead = 0;
    
    while (unitCamSerial.available() && bytesRead < 100) {
      buffer[bytesRead] = unitCamSerial.read();
      bytesRead++;
      delay(1);
    }
    
    if (bytesRead > 0) {
      Serial.println("Received " + String(bytesRead) + " bytes:");
      printHexData(buffer, bytesRead);
    }
  }
}

// Print data in hexadecimal format
void printHexData(uint8_t* data, int length) {
  Serial.print("  Hex: ");
  for (int i = 0; i < length; i++) {
    if (data[i] < 16) Serial.print("0");
    Serial.print(data[i], HEX);
    Serial.print(" ");
    if ((i + 1) % 16 == 0) {
      Serial.println();
      Serial.print("       ");
    }
  }
  Serial.println();
  
  // Also print as ASCII characters
  Serial.print("  ASCII: ");
  for (int i = 0; i < length; i++) {
    if (data[i] >= 32 && data[i] <= 126) {
      Serial.print((char)data[i]);
    } else {
      Serial.print(".");
    }
  }
  Serial.println();
}

// Streaming mode main loop
void streamCameraLoop() {
  static unsigned long lastCapture = 0;
  
  if (millis() - lastCapture >= STREAM_INTERVAL) {
    captureAndStreamImage();
    lastCapture = millis();
  }
  
  // Small delay to prevent overwhelming the system
  delay(10);
}

// Capture image and stream it over serial
void captureAndStreamImage() {
  // Send capture command
  unitCamSerial.println("AT+CAPTURE");
  delay(500); // Wait for camera to process
  
  // Allocate buffer for image data
  uint8_t* imageBuffer = (uint8_t*)malloc(MAX_IMAGE_SIZE);
  if (!imageBuffer) {
    Serial.println("ERROR: Could not allocate memory for image");
    return;
  }
  
  int bytesRead = 0;
  unsigned long startTime = millis();
  
  // Read image data from camera
  while (bytesRead < MAX_IMAGE_SIZE && (millis() - startTime) < 3000) {
    if (unitCamSerial.available()) {
      imageBuffer[bytesRead] = unitCamSerial.read();
      bytesRead++;
      startTime = millis(); // Reset timeout on data reception
    }
    delay(1);
  }
  
  if (bytesRead > 0) {
    // Send image over serial to PC
    sendImageOverSerial(imageBuffer, bytesRead);
  } else {
    Serial.println("FRAME_ERROR: No image data received");
  }
  
  free(imageBuffer);
}

// Send image data over serial in a format the PC can understand
void sendImageOverSerial(uint8_t* imageData, int imageSize) {
  // Send frame header
  Serial.print("FRAME_START:");
  Serial.print(imageSize);
  Serial.println();
  
  // Send image data in chunks to avoid buffer overflow
  const int chunkSize = 64;
  for (int i = 0; i < imageSize; i += chunkSize) {
    int remaining = imageSize - i;
    int currentChunk = (remaining < chunkSize) ? remaining : chunkSize;
    
    Serial.print("DATA:");
    for (int j = 0; j < currentChunk; j++) {
      if (imageData[i + j] < 16) Serial.print("0");
      Serial.print(imageData[i + j], HEX);
    }
    Serial.println();
    
    delay(1); // Small delay to prevent overwhelming serial buffer
  }
  
  // Send frame footer
  Serial.println("FRAME_END");
  Serial.flush(); // Ensure all data is sent
}