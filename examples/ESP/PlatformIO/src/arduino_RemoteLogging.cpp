#include <Arduino.h>

#define CHRONOLOG_REMOTE_ENABLE         1           // Enable remote logging server
#define CHRONOLOG_COLOR_ENABLE          1           // 0 = Disable colors for ANSII-incompatible terminals

#include "ChronoLog.h"

ChronoLogger logger("RemoteLogger", CHRONOLOG_LEVEL_DEBUG);

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  logger.debug("ESP32 Remote Logging Example Starting...");
  logger.info("Logger initialized successfully at DEBUG level");
}

void loop() {
  logger.debug("Temperature reading: 25.6°C");
  logger.info("Network status: Connected");
  logger.warn("High latency detected: 150ms");

  delay(2000);
}
