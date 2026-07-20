extern "C"  { 
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
}

#define CHRONOLOG_PRO_FEATURES          1           // Enable Pro features for progress bar
#define CHRONOLOG_COLOR_ENABLE          1           // 0 = Disable colors for ANSII-incompatible terminals

#include "ChronoLog.h"

ChronoLogger logger("ProgressBar", CHRONOLOG_LEVEL_PRO_FEATURES);

extern "C" void app_main(void) {
    /* 
      🕒 NOTE: If you sync system time to local time or UTC using NTP or another method,
      ChronoLog will automatically show real timestamps instead of MCU uptime-based logs 
    */

    logger.debug("ESP32 Progress Bar Example Starting...");
    logger.info("Logger initialized successfully at PRO_FEATURES level");

    while (true) { 
        logger.info("Simulating Firmware Update...");
        int total_chunks = 1000;
        
        for (int current = 0; current <= total_chunks; current++) {
            logger.progress(current, total_chunks, "Updating");
            
            /* Simulate work */
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        
        logger.info("Update Complete!");
        vTaskDelay(pdMS_TO_TICKS(5000));                                                // Progress Simulation every 5 seconds
    }  
}
