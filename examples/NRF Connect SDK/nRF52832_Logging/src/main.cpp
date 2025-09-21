extern "C" {
#include <stdio.h>
}

#include "ChronoLog.h"

ChronoLogger logger("myLogger", CHRONOLOG_LEVEL_DEBUG);

int main(void) {
	logger.debug("nRF52832 Logging Example Starting...");
    logger.info("Logger initialized successfully at DEBUG level");

	while (true) {
		logger.debug("This is a debug message");
        logger.info("This is an info message");
        logger.warn("This is a warning message");
        logger.error("This is an error message");
        logger.fatal("This is a fatal message");

		k_sleep(K_MSEC(5000));
	}

	return 0;
}
