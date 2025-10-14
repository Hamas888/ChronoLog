#include <iostream>
#define CHRONOLOG_PRO_FEATURES 1
#include "../include/ChronoLog.h"

using namespace std;

int main() {
    ChronoLogger logger("TestModule", CHRONOLOG_LEVEL_PRO_FEATURES);

    logger.debug("This is a debug message.");

    for(int i = 0; i <= 100; i += 10) {
        logger.progress(i, 100, "Processing");
        // Simulate some work
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }   
    return 0;
}