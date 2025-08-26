#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

// Create an instance of the BNO055 sensor
// The default I2C address is 0x28, but can also be 0x29
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

void setup(void) {
  // Start the serial communication for debugging
  Serial.begin(115200);
  while (!Serial); // wait for serial port to connect. Needed for native USB
  Serial.println("BNO055 Test");
  Serial.println("");

  // Initialize the BNO055 sensor
  if (!bno.begin()) {
    Serial.print("No BNO055 detected ... Check your wiring or I2C ADDR!");
    while (1);
  }

  // Optional: Display some basic information about the sensor
  sensor_t sensor;
  bno.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print("Sensor:       "); Serial.println(sensor.name);
  Serial.print("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.println("------------------------------------");

  delay(1000);

  // Set the sensor to operate in NDOF (Nine Degrees of Freedom) mode
  // This mode fuses accelerometer, gyroscope, and magnetometer data
  bno.setExtCrystalUse(true);
}

void loop(void) {
  // Request the orientation data (Euler angles)
  sensors_event_t event;
  bno.getEvent(&event);

  // Print the Euler angles (orientation)
  // event.orientation.x is the heading
  // event.orientation.y is the roll
  // event.orientation.z is the pitch
  Serial.print("Heading (X): ");
  Serial.print(event.orientation.x, 2); // Print with 2 decimal places
  Serial.print("\t  Roll (Y): ");
  Serial.print(event.orientation.y, 2);
  Serial.print("\t  Pitch (Z): ");
  Serial.println(event.orientation.z, 2);

  // Get and display the calibration status
  uint8_t system, gyro, accel, mag;
  system = gyro = accel = mag = 0;
  bno.getCalibration(&system, &gyro, &accel, &mag);
  
  Serial.print("Calibration: Sys=");
  Serial.print(system, DEC);
  Serial.print(" Gyro=");
  Serial.print(gyro, DEC);
  Serial.print(" Accel=");
  Serial.print(accel, DEC);
  Serial.print(" Mag=");
  Serial.println(mag, DEC);

  // A small delay before the next reading
  delay(100);
}