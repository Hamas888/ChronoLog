#include <Arduino.h>

#define CHRONOLOG_PRO_FEATURES 1
#include "ChronoLog.h"

ChronoLogger logger("myLogger", CHRONOLOG_LEVEL_PRO_FEATURES);

void setup() {
  Serial.begin(115200);
  while (!Serial) {}                                                          // wait for serial port to connect. Needed for native USB}

  /* 
    🕒 NOTE: If you sync system time to local time or UTC using NTP or another method,
    ChronoLog will automatically show real timestamps instead of MCU uptime-based logs 
  */

  logger.debug("ESP32-S3 Progress Bar Example Starting...");
  logger.info("Logger initialized successfully at PRO_FEATURES level");
}

void loop() {
  logger.info("Simulating Download...");
  int total_Chunks = 1000;
  for(int current=0; current <= total_Chunks; current++) {
    logger.progress(current, total_Chunks, "Downloading");
    /* Simulate work */
    delay(10);                                                                 // Simulate work
  }

  delay(5000);                                                                //  Progress Simulation every 5 seconds
}