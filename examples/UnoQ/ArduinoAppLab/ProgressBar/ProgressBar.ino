#include <zephyr/kernel.h>
#include <Arduino_RouterBridge.h>

#define CHRONOLOG_PRO_FEATURES          1           // Enable Pro features like progress bar
#define CHRONOLOG_COLOR_ENABLE          1           // 0 = Disable colors for ANSII-incompatible terminals

#include "ChronoLog.h"

ChronoLogger logger("myLogger", CHRONOLOG_LEVEL_PRO_FEATURES);

void setup() {
  Monitor.begin();

  logger.debug("Arduino Uno Q Progress Bar Example Starting...");
  logger.info("Logger initialized successfully at PRO_FEATURES level");
}

void loop() {
  logger.info("Simulating Download...");
  int total_Chunks = 1000;
  for(int current=0; current <= total_Chunks; current++) {
    logger.progress(current, total_Chunks, "Downloading");
    /* Simulate work */
    k_msleep(10);                                                                 // Simulate work
  }

  k_msleep(5000);                                                                 //  Progress Simulation every 5 seconds
}