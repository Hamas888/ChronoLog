#ifndef CHRONOLOG_FILE_SINK_H
#define CHRONOLOG_FILE_SINK_H

// Desktop rotating file sink.
//
// Writes one file per module to a configurable directory (default "logs/").
// - Condensation OFF by default: full-fidelity lines.
//   Set setCondensed(true) for the compact MCU-style format (|I| levels, thread registry).
// - Every file starts with a signature line so tools/chrono_dump.py can detect it.
// - Built-in recycling: age-based retention (days) + optional size-based rotation.

#include "ChronoLog.h"
#include "ChronoLogSink.h"

#if CHRONOLOG_SINKS_ENABLE && defined(CHRONOLOG_PLATFORM_DESKTOP)

#include <cstdio>
#include <cstring>
#include <ctime>

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
    #define CHRONOLOG_SINK_FILE_CONDENSED              0   // desktop: full-fidelity by default
  #endif
#endif

// The library signature every ChronoLog log file starts with (for chrono_dump.py).
#define CHRONOLOG_LOG_SIGNATURE                       "# chrono-log sig\n"

class ChronoLogFileSink : public ChronoLogSink {
public:
  static ChronoLogFileSink& instance();

  void setLogsDir(const char* path);       // default "logs"
  void setRetentionDays(int days);         // default 7
  void setMaxFileBytes(size_t bytes);      // default 0 = no cap
  void setRotations(int n);                // default 3
  void setCondensed(bool on);              // default false (full-fidelity)
  bool isCondensed() const { return condensed_; }

  // ChronoLogSink interface
  void write(const ChronoLogSinkLine& line) override;
  void flush() override;

private:
  ChronoLogFileSink();
  ~ChronoLogFileSink() override;
  ChronoLogFileSink(const ChronoLogFileSink&) = delete;
  ChronoLogFileSink& operator=(const ChronoLogFileSink&) = delete;

  void ensureOpen(const char* module);
  void checkRetention(FILE* f, const char* path);
  void checkRotation(const char* module);
  void writeHeader(FILE* f, const char* module);
  const char* levelShort(const char* level) const;

  char logsDir_[256];
  int retentionDays_;
  size_t maxFileBytes_;
  int rotations_;
  bool condensed_;

  FILE* currentFile_;
  char currentModule_[16];
  size_t currentSize_;

  struct ThreadEntry {
    char name[32];   // the actual task/thread name (for dedup)
    int id;          // short id, emitted as "T<id>"
  };
  ThreadEntry threads_[32];
  int threadCount_;

  bool headerWritten_;
};

#endif // CHRONOLOG_SINKS_ENABLE && CHRONOLOG_PLATFORM_DESKTOP
#endif // CHRONOLOG_FILE_SINK_H
