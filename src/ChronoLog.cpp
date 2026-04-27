/*
  ChronoLog.cpp
  Central definitions for ChronoLog's cross-translation-unit objects.
*/

#include "ChronoLog.h"

#if CHRONOLOG_THREAD_SAFE
  #if defined(CHRONOLOG_PLATFORM_DESKTOP)
    std::mutex chronoLogMutex;
  #elif defined(CHRONOLOG_STM32_FREERTOS)
    osMutexId_t chronoLogMutex = nullptr;
  #elif defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_ESP)
    SemaphoreHandle_t chronoLogMutex = nullptr;
  #elif defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_UNO_Q)
    struct k_mutex chronoLogMutex;
  #endif
#endif
