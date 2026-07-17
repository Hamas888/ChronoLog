#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define CHRONOLOG_THREAD_SAFE           1           // Enabled thread-safe logging
#define CHRONOLOG_COLOR_ENABLE          1           // 0 = Disable colors for ANSII-incompatible terminals

#include "ChronoLog.h"

ChronoLogger logger("ThreadSafeLogger", CHRONOLOG_LEVEL_DEBUG);

extern "C" void app_main(void) {
    logger.debug("ESP32 Thread Safety Example Starting...");
    logger.info("Logger initialized successfully at DEBUG level with Thread Safety ENABLED");

    while (true) {
        logger.debug("This is a debug message");
        logger.info("This is an info message");
        logger.warn("This is a warning message");
        logger.error("This is an error message");

        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}
