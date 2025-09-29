#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

extern const uint8_t BNO055_SDA_PIN;
extern const uint8_t BNO055_SCL_PIN;

extern const uint8_t GPS_RX_PIN;
extern const uint8_t GPS_TX_PIN;

extern const uint8_t ULTRASONIC_TRIGGER_PIN;
extern const uint8_t ULTRASONIC_ECHO_PIN;

extern const uint8_t SD_CD_PIN;
extern const uint8_t SD_CMD_PIN;
extern const uint8_t SD_CLK_PIN;
extern const uint8_t SD_DATA0_PIN;

extern const uint8_t MOTOR_STBY;
extern const uint8_t MOTOR_A_PWM;
extern const uint8_t MOTOR_A_IN1;
extern const uint8_t MOTOR_A_IN2;
extern const uint8_t MOTOR_B_PWM;
extern const uint8_t MOTOR_B_IN1;
extern const uint8_t MOTOR_B_IN2;

extern const uint8_t LED_PIN;

//operation related constants
extern const float CLOSEIN_START_THRESHOLD;
extern const float TARGET_REACHED_THRESHOLD;
extern const float LOG_QUEUE_SIZE;
extern const float MAX_LOG_LENGTH;
extern const float TASK0_HEAP_SIZE;
extern const float TASK1_HEAP_SIZE;
extern const float TASK2_HEAP_SIZE;
extern const float TASK_HEAP_BLOCK_SIZE;

//important variables
extern String state;
extern String state_description;

//task handles
extern TaskHandle_t task0Handle;
extern TaskHandle_t task1Handle;
extern TaskHandle_t task2Handle;

#endif // CONSTANTS_H