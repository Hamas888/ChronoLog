// Graph Plotter Example - demonstrates all plot() features.
// CHRONOLOG_PLOT_ANSI defaults to 1 on desktop (live multi-row chart) and
// 0 on bare UART (single-line sparkline). Define it explicitly to override.
#define CHRONOLOG_PRO_FEATURES          1           // Enable Pro features for graph plotter
#define CHRONOLOG_COLOR_ENABLE          1           // 0 = Disable colors for ANSII-incompatible terminals

#include "ChronoLog.h"
#include <thread>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>

int main() {
    ChronoLogger logger("GraphPlot", CHRONOLOG_LEVEL_PRO_FEATURES);

    logger.debug("Graph Plotter Example Starting...");
    logger.info("Logger initialized successfully at PRO_FEATURES level");

    // --- 1. Live chart: single-sample plot, redraws in place.
    // With CHRONOLOG_PLOT_ANSI=1 this renders a multi-row chart that updates live.
    // Set CHRONOLOG_PLOT_ANSI=0 to see the single-line sparkline fallback.
    logger.info("Plot Feature 1: Live chart (plot(series, value))");
    float phase = 0.0f;
    for (int i = 0; i < 40; i++) {
        float value = 20.0f + 5.0f * std::sin(phase);
        logger.plot("temp", value);
        phase += 0.35f;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));   // Simulate work
    }
    #if !CHRONOLOG_PLOT_ANSI
        printf("\n");   // Move past the single-line sparkline (ANSI chart already ends with \n)
    #endif

    // --- 2. Flat series edge case (max == min) is handled gracefully
    logger.info("Plot Feature 2: Flat series (constant value)");
    for (int i = 0; i < 12; i++) {
        logger.plot("steady", 42.0f);
    }
    #if !CHRONOLOG_PLOT_ANSI
        printf("\n");
    #endif

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

    // --- 5. Time-windowed batch: X-axis bucketed into time slices
    // plot(series, values, count, timeWindowSec=8) renders 8 time-bucketed columns.
    logger.info("Plot Feature 5: Time-windowed batch (plot(series, values[], count, timeWindowSec))");
    logger.plot("accelT", accel, 32, 8);

    // --- 6. Single-line sparkline fallback (CHRONOLOG_PLOT_ANSI=0)
    // Demonstrates the bare-UART / plain-serial-monitor mode: one line overwritten
    // with '\r' instead of a multi-row chart.
    logger.info("Plot Feature 6: Single-line fallback (CHRONOLOG_PLOT_ANSI=0)");
    #if CHRONOLOG_PLOT_ANSI
        logger.info("(CHRONOLOG_PLOT_ANSI=1 here, so live plot() used the multi-row chart above.)");
    #else
        phase = 0.0f;
        for (int i = 0; i < 24; i++) {
            float value = 20.0f + 5.0f * std::sin(phase);
            logger.plot("spark", value);
            phase += 0.35f;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        printf("\n");
    #endif

    logger.info("Graph Plotter Example Finished");
    return 0;
}
