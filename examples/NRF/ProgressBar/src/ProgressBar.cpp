#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define CHRONOLOG_PRO_FEATURES          1           // Enable Pro features like progress bar
#define CHRONOLOG_COLOR_ENABLE          1           // 0 = Disable colors for ANSII-incompatible terminals

#include "ChronoLog.h"

ChronoLogger logger("ProgressBar", CHRONOLOG_LEVEL_PRO_FEATURES);

int main(void) {
    logger.debug("NRF Zephyr Progress Bar Example Starting...");
    logger.info("Logger initialized successfully at PRO_FEATURES level");

    while (true) {
        logger.info("Simulating Firmware Update...");
        int total_chunks = 100;
        
        for (int current = 0; current <= total_chunks; current++) {
            logger.progress(current, total_chunks, "Updating");
            
            // Simulate work
            k_msleep(50);
        }

        logger.info("Update Complete!");
        k_msleep(5000); // Wait 5 seconds before repeating
    }

    return 0;
}
