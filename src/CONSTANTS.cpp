#include "CONSTANTS.h"
#include <Arduino.h>

const uint8_t BNO055_SDA_PIN = 7; // D0
const uint8_t BNO055_SCL_PIN  = 8; // D1

const uint8_t GPS_RX_PIN = 10; // D3
const uint8_t GPS_TX_PIN = 9;  // D2

const uint8_t ULTRASONIC_TRIGGER_PIN = 1;
const uint8_t ULTRASONIC_ECHO_PIN = 3;

const uint8_t SD_CD_PIN      = 13;
const uint8_t SD_CMD_PIN     = 23;
const uint8_t SD_CLK_PIN     = 18;
const uint8_t SD_DATA0_PIN   = 19;

const uint8_t MOTOR_STBY    = 15;    // Standby pin - LOW = standby, HIGH = active
const uint8_t MOTOR_A_PWM   = 27;    // Motor A PWM (speed control)
const uint8_t MOTOR_A_IN1   = 12;    // Motor A direction pin 1
const uint8_t MOTOR_A_IN2   = 14;    // Motor A direction pin 2
const uint8_t MOTOR_B_PWM   = 33;    // Motor B PWM (speed control)
const uint8_t MOTOR_B_IN1   = 2;     // Motor B direction pin 1
const uint8_t MOTOR_B_IN2   = 4;     // Motor B direction pin 2

const uint8_t LED_PIN = 32;

//operation related constants
extern const float CLOSEIN_START_THRESHOLD = 5.0; // in meters
extern const float TARGET_REACHED_THRESHOLD = 2.0; // in meters

//important variables
extern String state = "";
extern String state_description = "";
extern const float LOG_QUEUE_SIZE = 50;
extern const float MAX_LOG_LENGTH = 256;

//task handles
extern TaskHandle_t task0Handle = nullptr;
extern TaskHandle_t task1Handle = nullptr;
extern TaskHandle_t task2Handle = nullptr;