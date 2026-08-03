/*
  ChronoLogSink.cpp
  Implementation of the sink registry singleton.
*/

#include "ChronoLog.h"
#include "ChronoLogSink.h"

#if CHRONOLOG_SINKS_ENABLE

ChronoLogSinkRegistry::ChronoLogSinkRegistry()
  #if !defined(CHRONOLOG_PLATFORM_DESKTOP) && !defined(CHRONOLOG_PLATFORM_ESP_IDF) && !defined(CHRONOLOG_ESP)
    : sinkCount_(0)
  #endif
{
  #if !defined(CHRONOLOG_PLATFORM_DESKTOP) && !defined(CHRONOLOG_PLATFORM_ESP_IDF) && !defined(CHRONOLOG_ESP)
    for (int i = 0; i < CHRONOLOG_MAX_SINKS; i++) sinks_[i] = NULL;
  #endif
}

ChronoLogSinkRegistry& ChronoLogSinkRegistry::instance() {
  static ChronoLogSinkRegistry s_instance;
  return s_instance;
}

void ChronoLogSinkRegistry::add(ChronoLogSink* sink) {
  if (!sink) return;
  #if CHRONOLOG_THREAD_SAFE
    ChronoLogger::threadSafeLock();
  #endif
  #if defined(CHRONOLOG_PLATFORM_DESKTOP) || defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_ESP)
    for (size_t i = 0; i < sinks_.size(); i++) {
      if (sinks_[i] == sink) {
        #if CHRONOLOG_THREAD_SAFE
          ChronoLogger::threadSafeUnlock();
        #endif
        return;
      }
    }
    sinks_.push_back(sink);
  #else
    for (int i = 0; i < sinkCount_; i++) {
      if (sinks_[i] == sink) {
        #if CHRONOLOG_THREAD_SAFE
          ChronoLogger::threadSafeUnlock();
        #endif
        return;
      }
    }
    if (sinkCount_ < CHRONOLOG_MAX_SINKS) {
      sinks_[sinkCount_++] = sink;
    }
  #endif
  #if CHRONOLOG_THREAD_SAFE
    ChronoLogger::threadSafeUnlock();
  #endif
}

void ChronoLogSinkRegistry::remove(ChronoLogSink* sink) {
  if (!sink) return;
  #if CHRONOLOG_THREAD_SAFE
    ChronoLogger::threadSafeLock();
  #endif
  #if defined(CHRONOLOG_PLATFORM_DESKTOP) || defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_ESP)
    for (size_t i = 0; i < sinks_.size(); i++) {
      if (sinks_[i] == sink) {
        sinks_.erase(sinks_.begin() + i);
        break;
      }
    }
  #else
    for (int i = 0; i < sinkCount_; i++) {
      if (sinks_[i] == sink) {
        for (int j = i; j < sinkCount_ - 1; j++) sinks_[j] = sinks_[j + 1];
        sinkCount_--;
        break;
      }
    }
  #endif
  #if CHRONOLOG_THREAD_SAFE
    ChronoLogger::threadSafeUnlock();
  #endif
}

void ChronoLogSinkRegistry::flushAll() {
  #if CHRONOLOG_THREAD_SAFE
    ChronoLogger::threadSafeLock();
  #endif
  #if defined(CHRONOLOG_PLATFORM_DESKTOP) || defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_ESP)
    for (size_t i = 0; i < sinks_.size(); i++) {
      if (sinks_[i]) sinks_[i]->flush();
    }
  #else
    for (int i = 0; i < sinkCount_; i++) {
      if (sinks_[i]) sinks_[i]->flush();
    }
  #endif
  #if CHRONOLOG_THREAD_SAFE
    ChronoLogger::threadSafeUnlock();
  #endif
}

void ChronoLogSinkRegistry::writeAll(const ChronoLogSinkLine& line) {
  #if CHRONOLOG_THREAD_SAFE
    // writeAll() is called from print() AFTER the main lock is released, so it's
    // safe to lock here. Sinks must not call back into ChronoLogger.
    ChronoLogger::threadSafeLock();
  #endif
  #if defined(CHRONOLOG_PLATFORM_DESKTOP) || defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_ESP)
    for (size_t i = 0; i < sinks_.size(); i++) {
      if (sinks_[i]) sinks_[i]->write(line);
    }
  #else
    for (int i = 0; i < sinkCount_; i++) {
      if (sinks_[i]) sinks_[i]->write(line);
    }
  #endif
  #if CHRONOLOG_THREAD_SAFE
    ChronoLogger::threadSafeUnlock();
  #endif
}

#endif // CHRONOLOG_SINKS_ENABLE
