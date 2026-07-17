#define CHRONOLOG_PRO_FEATURES          1           // Enable Pro features for progress bar
#define CHRONOLOG_COLOR_ENABLE          1           // 0 = Disable colors for ANSII-incompatible terminals

#include <Arduino.h>
#include "ChronoLog.h"

ChronoLogger logger("myLogger", CHRONOLOG_LEVEL_PRO_FEATURES);

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  logger.debug("ESP32 Progress Bar Example Starting...");
  logger.info("Logger initialized successfully at PRO_FEATURES level");
}

void loop() {
  logger.info("Simulating Download...");
  int total_Chunks = 100;
  for(int current=0; current <= total_Chunks; current++) {
    logger.progress(current, total_Chunks, "Downloading");
    delay(10);
  }

  delay(5000);
}
