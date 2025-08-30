#ifndef LED_H
#define LED_H

#include <Arduino.h>

/**
 * @brief Initializes the LED system with the specified pin.
 * @param pin The GPIO pin number where the LED is connected.
 * @return true if initialization was successful, false otherwise.
 */
bool setupLed(int pin);

/**
 * @brief Sets the LED blinking pattern based on a string command.
 * @param state String command that defines the blinking pattern.
 * 
 * Supported commands:
 * - "OFF" or "off" - Turn LED off
 * - "ON" or "on" - Turn LED on (solid)
 * - "SLOW" or "slow" - Slow blink (1 second on, 1 second off)
 * - "FAST" or "fast" - Fast blink (200ms on, 200ms off)
 * - "PULSE" or "pulse" - Pulse pattern (fade in/out effect simulation)
 * - "SOS" or "sos" - SOS morse code pattern
 * - "HEARTBEAT" or "heartbeat" - Double pulse pattern
 * - "CUSTOM:XXX:YYY" - Custom pattern (XXX=on time, YYY=off time in ms)
 */
void setLedState(String state);

/**
 * @brief Sets the LED to display a custom Morse code sequence.
 * @param morseSequence String containing dots (.) and dashes (-) for Morse code.
 * 
 * Example: ".--.." for the letter 'P'
 * - Dot (.) = 200ms on, 200ms off
 * - Dash (-) = 600ms on, 200ms off
 * - Space between characters = 400ms off (additional)
 */
void setMorseCode(String morseSequence);

/**
 * @brief Updates the LED state. Must be called regularly in the main loop.
 * This function handles the timing and state changes for blinking patterns.
 */
void updateLed();

/**
 * @brief Gets the current LED state/pattern.
 * @return String describing the current LED pattern.
 */
String getLedState();

#endif // LED_H