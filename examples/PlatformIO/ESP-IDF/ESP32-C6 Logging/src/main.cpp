#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ChronoLog.h"

ChronoLogger logger("myLogger", CHRONOLOG_LEVEL_DEBUG);

extern "C" void app_main(void) {
    logger.debug("ESP32-S3 Logging Example Starting...");
    logger.info("Logger initialized successfully at DEBUG level");

    while (true) { 
        logger.debug("This is a debug message");
        logger.info("This is an info message");
        logger.warn("This is a warning message");
        logger.error("This is an error message");
        logger.fatal("This is a fatal message");
        
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }  
}