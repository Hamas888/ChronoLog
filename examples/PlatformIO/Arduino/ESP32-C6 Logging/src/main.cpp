#include <Arduino.h>
#include "ChronoLog.h"

ChronoLogger logger("myLogger", CHRONOLOG_LEVEL_DEBUG);

void setup() {
  Serial.begin(115200);
  while (!Serial) {}                                                          // wait for serial port to connect. Needed for native USB}

  /* 
    🕒 NOTE: If you sync system time to local time or UTC using NTP or another method,
    ChronoLog will automatically show real timestamps instead of MCU uptime-based logs 
  */

  logger.debug("ESP32-C6 Logging Example Starting...");
  logger.info("Logger initialized successfully at DEBUG level");
}

void loop() {
  logger.debug("This is a debug message");
  logger.info("This is an info message");
  logger.warn("This is a warning message");
  logger.error("This is an error message");
  logger.fatal("This is a fatal message");

  delay(5000);                                                                // Log messages every 5 seconds
}