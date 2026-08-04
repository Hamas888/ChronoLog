#ifndef CHRONOLOG_LITTLEFS_SINK_H
#define CHRONOLOG_LITTLEFS_SINK_H

// MCU file sink - LittleFS (ESP-IDF and Arduino-ESP32 / PlatformIO).
//
// Writes compacted logs to flash (condensation ON by default) with the same
// signature header, retention/rotation, and optional UART fetch responder.
//
// Platform support:
//   - ESP-IDF native: mounts LittleFS on a partition, uses esp_vfs_fs file APIs.
//   - Arduino-ESP32 (PlatformIO, framework = arduino): uses the Arduino <LittleFS.h>
//     core API (LittleFS.begin() / open / write).
//   - STM32/NRF and other non-ESP MCUs: not built-in yet - implement a ChronoLogSink
//     subclass instead (see examples/STM32 or README).

#include "ChronoLog.h"
#include "ChronoLogSink.h"

#if CHRONOLOG_SINKS_ENABLE && (defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_ESP))

#include <cstdio>
#include <cstring>

#if defined(CHRONOLOG_PLATFORM_ESP_IDF)
  #include "esp_log.h"
  #include "esp_littlefs.h"
  #include "esp_vfs_fs.h"
#elif defined(CHRONOLOG_PLATFORM_ARDUINO) && defined(CHRONOLOG_ESP)
  #include <LittleFS.h>
  #include <FS.h>
#endif

// Config defaults (override via build flags / Kconfig).
#ifndef CHRONOLOG_SINK_FILE_RETENTION_DAYS
  #ifdef CONFIG_CHRONOLOG_SINK_FILE_RETENTION_DAYS
    #define CHRONOLOG_SINK_FILE_RETENTION_DAYS         CONFIG_CHRONOLOG_SINK_FILE_RETENTION_DAYS
  #else
    #define CHRONOLOG_SINK_FILE_RETENTION_DAYS         7
  #endif
#endif

#ifndef CHRONOLOG_SINK_FILE_MAX_BYTES
  #ifdef CONFIG_CHRONOLOG_SINK_FILE_MAX_BYTES
    #define CHRONOLOG_SINK_FILE_MAX_BYTES              CONFIG_CHRONOLOG_SINK_FILE_MAX_BYTES
  #else
    #define CHRONOLOG_SINK_FILE_MAX_BYTES              0   // 0 = no size cap
  #endif
#endif

#ifndef CHRONOLOG_SINK_FILE_ROTATIONS
  #ifdef CONFIG_CHRONOLOG_SINK_FILE_ROTATIONS
    #define CHRONOLOG_SINK_FILE_ROTATIONS              CONFIG_CHRONOLOG_SINK_FILE_ROTATIONS
  #else
    #define CHRONOLOG_SINK_FILE_ROTATIONS              3
  #endif
#endif

#ifndef CHRONOLOG_SINK_FILE_CONDENSED
  #ifdef CONFIG_CHRONOLOG_SINK_FILE_CONDENSED
    #define CHRONOLOG_SINK_FILE_CONDENSED              CONFIG_CHRONOLOG_SINK_FILE_CONDENSED
  #else
    #define CHRONOLOG_SINK_FILE_CONDENSED              1   // MCU: compact by default
  #endif
#endif

#define CHRONOLOG_LOG_SIGNATURE                       "# chrono-log sig\n"

// Optional time provider: call setTimeProvider() if the app has an RTC/NTP clock,
// enabling age-based retention. Otherwise rotation is size-cap-only.
typedef uint32_t (*ChronoLogTimeProvider)();

class ChronoLogLittleFsSink : public ChronoLogSink {
public:
  static ChronoLogLittleFsSink& instance();

  bool begin();                          // mount LittleFS (format-on-fail once)
  void setMaxFileBytes(size_t bytes);
  void setRotations(int n);
  void setCondensed(bool on);
  void setTimeProvider(ChronoLogTimeProvider provider);  // enables age-based retention

  void pollFetch();                     // check UART for fetch command, stream logs if requested

  // ChronoLogSink interface
  void write(const ChronoLogSinkLine& line) override;
  void flush() override;

private:
  ChronoLogLittleFsSink();
  ~ChronoLogLittleFsSink() override;
  ChronoLogLittleFsSink(const ChronoLogLittleFsSink&) = delete;
  ChronoLogLittleFsSink& operator=(const ChronoLogLittleFsSink&) = delete;

  void checkRotation(const char* module);
  void writeHeader(void* file, const char* module);
  const char* levelShort(const char* level) const;
  void writeFileInternal(const char* module, const char* line);
  void sendFileOverUart(const char* module);

  bool mounted_;
  size_t maxFileBytes_;
  int rotations_;
  bool condensed_;
  ChronoLogTimeProvider timeProvider_;

  char currentModule_[16];
  size_t currentSize_;
};

#endif // CHRONOLOG_SINKS_ENABLE && (ESP_IDF || ESP)
#endif // CHRONOLOG_LITTLEFS_SINK_H
