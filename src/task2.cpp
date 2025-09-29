// Write to SD card Task
// On core 0 (BLOCKING)

#include <Arduino.h>
#include "sd_logger.h"
#include "task0.h"
#include "CONSTANTS.h"

// Global queue handle for inter-task communication
static QueueHandle_t logQueue = NULL;

bool initializeLogQueue() {
  logQueue = xQueueCreate(LOG_QUEUE_SIZE, MAX_LOG_LENGTH);
  if (logQueue == NULL) {
    return false;
  } else { 
    return true;
  }
}
