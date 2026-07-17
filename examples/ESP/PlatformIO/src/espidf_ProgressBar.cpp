#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define CHRONOLOG_PRO_FEATURES          1           // Enable Pro features for progress bar
#define CHRONOLOG_COLOR_ENABLE          1           // 0 = Disable colors for ANSII-incompatible terminals

#include "ChronoLog.h"

ChronoLogger logger("myLogger", CHRONOLOG_LEVEL_PRO_FEATURES);

extern "C" void app_main(void) {
    logger.debug("ESP32 Progress Bar Example Starting...");
    logger.info("Logger initialized successfully at PRO_FEATURES level");

    while (true) {
        logger.info("Simulating Download...");
        int total_Chunks = 100;
        for(int current=0; current <= total_Chunks; current++) {
            logger.progress(current, total_Chunks, "Downloading");
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
