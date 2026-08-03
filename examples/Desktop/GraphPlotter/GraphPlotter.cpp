#define CHRONOLOG_PRO_FEATURES          1           // Enable Pro features for graph plotter
#define CHRONOLOG_COLOR_ENABLE          1           // 0 = Disable colors for ANSII-incompatible terminals

#include "ChronoLog.h"
#include <thread>
#include <chrono>
#include <cmath>

int main() {
    ChronoLogger logger("GraphPlot", CHRONOLOG_LEVEL_PRO_FEATURES);

    logger.debug("Graph Plotter Example Starting...");
    logger.info("Logger initialized successfully at PRO_FEATURES level");

    // --- 1. Live sparkline: push one sample at a time, updates in place with '\r'
    logger.info("Plot Feature 1: Live sparkline (plot(series, value))");
    float phase = 0.0f;
    for (int i = 0; i < 40; i++) {
        float value = 20.0f + 5.0f * std::sin(phase);
        logger.plot("temp", value);
        phase += 0.35f;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));   // Simulate work
    }
    printf("\n");                                                       // Move past the sparkline line

    // --- 2. Flat series edge case (max == min) is handled gracefully
    logger.info("Plot Feature 2: Flat series (constant value)");
    for (int i = 0; i < 12; i++) {
        logger.plot("steady", 42.0f);
    }
    printf("\n");

    // --- 3. Batch plot: push multiple values at once, renders a full window chart
    logger.info("Plot Feature 3: Batch window chart (plot(series, values[], count))");
    float accel[32];
    for (int i = 0; i < 32; i++) {
        accel[i] = 100.0f + 40.0f * std::sin(i * 0.4f) + 10.0f * std::cos(i * 1.1f);
    }
    logger.plot("accel", accel, 32);

    // --- 4. On-demand window re-render: one series, then all registered series
    logger.info("Plot Feature 4: On-demand re-render (plotWindow(series) then plotWindow())");
    logger.plotWindow("accel");
    logger.plotWindow();

    logger.info("Graph Plotter Example Finished");
    return 0;
}
