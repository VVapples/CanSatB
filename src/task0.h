#ifndef TASK0_H
#define TASK0_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

void task0Loop(void *pvParameters);

#endif // TASK0_H