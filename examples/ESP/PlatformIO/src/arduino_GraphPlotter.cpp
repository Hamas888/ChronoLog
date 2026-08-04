// Graph Plotter Example - Arduino framework + PlatformIO (ESP32).
//
// Demonstrates plot() on a bare-UART serial monitor:
// - live single-line sparkline (CHRONOLOG_PLOT_ANSI=0 default on MCU/UART)
// - flat-series edge case
// - batch window chart
// - on-demand re-render (plotWindow)
// - time-windowed batch
//
// Use a serial monitor that handles '\r' for the live sparkline.

#define CHRONOLOG_PRO_FEATURES          1           // Enable Pro features for graph plotter
#define CHRONOLOG_COLOR_ENABLE          1           // 0 = Disable colors for ANSII-incompatible terminals

#include <Arduino.h>
#include "ChronoLog.h"
#include <math.h>

ChronoLogger logger("GraphPlot", CHRONOLOG_LEVEL_PRO_FEATURES);

void setup() {
  Serial.begin(115200);
  while (!Serial) {}                                                          // wait for serial port to connect

  logger.debug("ESP32 Graph Plotter Example Starting...");
  logger.info("Logger initialized successfully at PRO_FEATURES level");
}

void loop() {
  // --- 1. Live sparkline: single-sample plot, overwrites in place with '\r'
  logger.info("Plot Feature 1: Live sparkline (plot(series, value))");
  float phase = 0.0f;
  for (int i = 0; i < 40; i++) {
    float value = 20.0f + 5.0f * sinf(phase);
    logger.plot("temp", value);
    phase += 0.35f;
    delay(20);
  }
  Serial.println();

  // --- 2. Flat series edge case (max == min) handled gracefully
  logger.info("Plot Feature 2: Flat series (constant value)");
  for (int i = 0; i < 12; i++) {
    logger.plot("steady", 42.0f);
    delay(10);
  }
  Serial.println();

  // --- 3. Batch plot: push many samples, renders a full window chart
  logger.info("Plot Feature 3: Batch window chart (plot(series, values[], count))");
  float accel[32];
  for (int i = 0; i < 32; i++) {
    accel[i] = 100.0f + 40.0f * sinf(i * 0.4f) + 10.0f * cosf(i * 1.1f);
  }
  logger.plot("accel", accel, 32);

  // --- 4. On-demand re-render: one series, then all
  logger.info("Plot Feature 4: On-demand re-render (plotWindow)");
  logger.plotWindow("accel");
  logger.plotWindow();

  // --- 5. Time-windowed batch: 8 time-bucketed columns
  logger.info("Plot Feature 5: Time-windowed batch (plot(series, values[], count, timeWindowSec))");
  logger.plot("accelT", accel, 32, 8);

  logger.info("Graph Plotter Example Finished - next pass in 5s");
  delay(5000);
}
