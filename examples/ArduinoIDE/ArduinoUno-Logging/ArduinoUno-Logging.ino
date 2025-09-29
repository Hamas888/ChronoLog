#include "ChronoLog.h"

ChronoLogger logger("myLogger", CHRONOLOG_LEVEL_DEBUG);

void setup() {
  Serial.begin(115200);

  logger.debug("Arduino Uno Logging Example Starting...");
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