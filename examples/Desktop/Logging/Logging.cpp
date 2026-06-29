#define CHRONOLOG_COLOR_ENABLE          1           // 0 = Disable colors for ANSII-incompatible terminals

#include "ChronoLog.h"
#include <thread>
#include <chrono>

int main() {
    ChronoLogger logger("myLogger", CHRONOLOG_LEVEL_DEBUG);

    logger.debug("Desktop Logging Example Starting...");
    logger.info("Logger initialized successfully at DEBUG level");

    for (int i = 0; i < 3; i++) {
        logger.debug("This is a debug message");
        logger.info("This is an info message");
        logger.warn("This is a warning message");
        logger.error("This is an error message");
        logger.fatal("This is a fatal message");

        std::this_thread::sleep_for(std::chrono::milliseconds(100));           // Small delay between batches
    }

    return 0;
}
