#define CHRONOLOG_THREAD_SAFE           1           // Enabled thread-safe logging
#define CHRONOLOG_COLOR_ENABLE          1           // 0 = Disable colors for ANSII-incompatible terminals

#include <Arduino.h>
#include "ChronoLog.h"

ChronoLogger logger("ThreadSafeLogger", CHRONOLOG_LEVEL_DEBUG);

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  logger.debug("ESP32 Thread Safety Example Starting...");
  logger.info("Logger initialized successfully at DEBUG level with Thread Safety ENABLED");
}

void loop() {
  logger.debug("This is a debug message from loop");
  logger.info("This is an info message from loop");
  logger.warn("This is a warning message");
  logger.error("This is an error message");

  delay(3000);
}
