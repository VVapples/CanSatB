#ifndef LED_H
#define LED_H

#include <Arduino.h>

void setupLed(int pin);
void dot();
void dash();
void ledMessage(String message);

#endif // LED_H