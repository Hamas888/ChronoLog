// FileSink Example - Arduino framework + PlatformIO (ESP32).
//
// Demonstrates the LittleFS file sink on the ESP32 Arduino core:
// logs are written compacted to flash, and can be fetched over UART with
// tools/chrono_dump.py --port <port> --baud <baud>.
//
// Requires in platformio.ini:
//   build_flags = -DCHRONOLOG_SINKS_ENABLE=1
// and a partition table with a spiffs/littlefs partition (see platformio.ini).

#include <Arduino.h>

#define CHRONOLOG_COLOR_ENABLE          1           // 0 = Disable colors for ANSII-incompatible terminals
#define CHRONOLOG_SINKS_ENABLE          1           // Enable sink interface + LittleFS sink

#include "ChronoLog.h"
#include "ChronoLogSink.h"
#include "ChronoLogLittleFsSink.h"

ChronoLogger logger("FileSink", CHRONOLOG_LEVEL_DEBUG);

void setup() {
  Serial.begin(115200);
  while (!Serial) {}                                                          // wait for serial port to connect

  // Mount LittleFS (format-on-fail once) and register the file sink.
  ChronoLogLittleFsSink::instance().begin();
  ChronoLogSinkRegistry::instance().add(&ChronoLogLittleFsSink::instance());

  // Optional: provide a time source so age-based retention works.
  // ChronoLogLittleFsSink::instance().setTimeProvider(my_rtc_uptime_ms);

  logger.info("LittleFS file sink started (condensed by default)");
}

void loop() {
  logger.debug("sensor tick %d", millis() % 1000);
  logger.info("temperature %.2f C", 20.0f + 0.1f * (float)(millis() % 500));
  delay(2000);
}
