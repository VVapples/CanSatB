#ifndef TASK2_H
#define TASK2_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

bool initializeLogQueue();
void task2Loop(void *pvParameters); 

#endif // TASK2_H