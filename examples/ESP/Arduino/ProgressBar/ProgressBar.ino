#define CHRONOLOG_PRO_FEATURES          1           // Enable Pro features for progress bar
#define CHRONOLOG_COLOR_ENABLE          1           // 0 = Disable colors for ANSII-incompatible terminals

#include "ChronoLog.h"

ChronoLogger logger("myLogger", CHRONOLOG_LEVEL_PRO_FEATURES);

void setup() {
  Serial.begin(115200);

  logger.debug("Arduino Progress Bar Example Starting...");
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

  delay(5000);                                                                 //  Progress Simulation every 5 seconds
}