#define CHRONOLOG_PRO_FEATURES          1           // Enable Pro features for progress bar
#define CHRONOLOG_COLOR_ENABLE          1           // 0 = Disable colors for ANSII-incompatible terminals

#include "ChronoLog.h"
#include <thread>
#include <chrono>

int main() {
    ChronoLogger logger("myLogger", CHRONOLOG_LEVEL_PRO_FEATURES);

    logger.debug("Desktop Progress Bar Example Starting...");
    logger.info("Logger initialized successfully at PRO_FEATURES level");

    logger.info("Simulating Download...");
    int total_Chunks = 50;
    for(int current=0; current <= total_Chunks; current++) {
        logger.progress(current, total_Chunks, "Downloading");
        /* Simulate work */
        std::this_thread::sleep_for(std::chrono::milliseconds(10));             // Simulate work
    }

    logger.info("Download complete!");
    return 0;
}
