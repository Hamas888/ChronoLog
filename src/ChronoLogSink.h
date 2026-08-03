#ifndef CHRONOLOG_SINK_H
#define CHRONOLOG_SINK_H

// Log Sink Interface - pluggable output sinks for ChronoLog.
//
// A sink receives every fully-formatted log line (after console + remote output)
// and can route it anywhere: file, network, flash, custom systems.
//
// Gated by CHRONOLOG_SINKS_ENABLE (default 0). Enable with:
//   #define CHRONOLOG_SINKS_ENABLE 1

#include "ChronoLog.h"

#ifndef CHRONOLOG_SINKS_ENABLE
  #ifdef CONFIG_CHRONOLOG_SINKS_ENABLE
    #define CHRONOLOG_SINKS_ENABLE                     CONFIG_CHRONOLOG_SINKS_ENABLE
  #else
    #define CHRONOLOG_SINKS_ENABLE                     0                                                              // Set to 1 to enable sink interface
  #endif
#endif // CHRONOLOG_SINKS_ENABLE

#ifndef CHRONOLOG_MAX_SINKS
  #ifdef CONFIG_CHRONOLOG_MAX_SINKS
    #define CHRONOLOG_MAX_SINKS                        CONFIG_CHRONOLOG_MAX_SINKS
  #else
    #define CHRONOLOG_MAX_SINKS                        4                                                               // Max registered sinks (MCU fixed array)
  #endif
#endif // CHRONOLOG_MAX_SINKS

#if CHRONOLOG_SINKS_ENABLE

// Structured log-line context delivered to every sink.
struct ChronoLogSinkLine {
  const char* time;      // "HH:MM:SS"
  const char* module;    // module name (the ChronoLogger name)
  const char* level;     // "INFO", "DEBUG", ... (unabbreviated)
  const char* task;      // thread/task name (may be "")
  const char* message;   // the formatted message
};

// Base class for a log sink. Implement write() to consume log lines.
class ChronoLogSink {
public:
  virtual ~ChronoLogSink() {}
  virtual void write(const ChronoLogSinkLine& line) = 0;
  virtual void flush() {}   // optional: flush any buffered output
};

// Global sink registry (mirrors ChronoLogRemote singleton pattern).
// Thread-safe via the existing chronoLogMutex (dispatch happens inside print()).
class ChronoLogSinkRegistry {
public:
  static ChronoLogSinkRegistry& instance();

  void add(ChronoLogSink* sink);
  void remove(ChronoLogSink* sink);
  void flushAll();
  void writeAll(const ChronoLogSinkLine& line);

private:
  ChronoLogSinkRegistry();

  // Desktop/ESP use std::vector (already available via ChronoLogRemote.h).
  // MCU uses a fixed array to avoid dynamic allocation.
  #if defined(CHRONOLOG_PLATFORM_DESKTOP) || defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_ESP)
    #include <vector>
    std::vector<ChronoLogSink*> sinks_;
  #else
    ChronoLogSink* sinks_[CHRONOLOG_MAX_SINKS];
    int sinkCount_;
  #endif
};

#endif // CHRONOLOG_SINKS_ENABLE
#endif // CHRONOLOG_SINK_H
