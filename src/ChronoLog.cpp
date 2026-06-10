/*
  ChronoLog.cpp
  Central definitions for ChronoLog's cross-translation-unit objects.
*/

#include "ChronoLog.h"

// Scope platform-specific macros to this translation unit only
#if defined(CHRONOLOG_PLATFORM_ARDUINO) && defined(CHRONOLOG_UNO_Q)
  #define strncat     chronolog_strncat
  #define vsnprintf   chronolog_vsnprintf
#endif

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

#if defined(CHRONOLOG_PLATFORM_ARDUINO) && defined(CHRONOLOG_UNO_Q)
  ThreadMapEntry threadsMap[CHRONOLOG_MAX_THREADS];
  int threadsMapCount = 0;
#endif

#if CHRONOLOG_MODE

ChronoLogger::ChronoLogger(const char* moduleName, ChronoLogLevel level)
  : name(moduleName), chronoLogLevel(level) {
  #if CHRONOLOG_THREAD_SAFE
    #if defined(CHRONOLOG_PLATFORM_STM32_HAL) && defined(CHRONOLOG_STM32_FREERTOS)
      if (chronoLogMutex == nullptr)  chronoLogMutex = osMutexNew(NULL);
    #elif defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_ESP)
      static portMUX_TYPE chronoLogMux = portMUX_INITIALIZER_UNLOCKED;
      if (chronoLogMutex == nullptr) {
        taskENTER_CRITICAL(&chronoLogMux);
        if (chronoLogMutex == nullptr)
          chronoLogMutex = xSemaphoreCreateMutex();
        taskEXIT_CRITICAL(&chronoLogMux);
      }
    #elif defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_UNO_Q)
      static bool initialized = false;
      if (!initialized) {
        k_mutex_init(&chronoLogMutex);
        initialized = true;
      }
    #endif
  #endif
}

#if defined(CHRONOLOG_UNO_Q)
  void ChronoLogger::setThreadName(const char* name) {
    k_tid_t current = k_sched_current_thread_query(); 
            
      for (int i = 0; i < threadsMapCount; i++) {
        if (threadsMap[i].id == current) {
          threadsMap[i].name = name;
          return;
        }
      }
      
      if (threadsMapCount < CHRONOLOG_MAX_THREADS) {
        threadsMap[threadsMapCount].id = current;
        threadsMap[threadsMapCount].name = name;
        threadsMapCount++;
      }
  }
#endif // CHRONOLOG_UNO_Q

#if CHRONOLOG_THREAD_SAFE
  void ChronoLogger::threadSafeLock() {
    #if defined(CHRONOLOG_PLATFORM_STM32_HAL) && defined(CHRONOLOG_STM32_FREERTOS)
      if (chronoLogMutex) osMutexAcquire(chronoLogMutex, portMAX_DELAY);
    #elif defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_ESP)
      if (chronoLogMutex) xSemaphoreTake(chronoLogMutex, portMAX_DELAY);
    #elif defined(CHRONOLOG_PLATFORM_ZEPHYR)|| defined(CHRONOLOG_UNO_Q)
      k_mutex_lock(&chronoLogMutex, K_FOREVER);
    #elif defined(CHRONOLOG_PLATFORM_DESKTOP)
      chronoLogMutex.lock();
    #endif
  }

  void ChronoLogger::threadSafeUnlock() {
    #if defined(CHRONOLOG_PLATFORM_STM32_HAL) && defined(CHRONOLOG_STM32_FREERTOS)
      if (chronoLogMutex) osMutexRelease(chronoLogMutex);
    #elif defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_ESP)
      if (chronoLogMutex) xSemaphoreGive(chronoLogMutex);
    #elif defined(CHRONOLOG_PLATFORM_ZEPHYR)|| defined(CHRONOLOG_UNO_Q)
      k_mutex_unlock(&chronoLogMutex);
    #elif defined(CHRONOLOG_PLATFORM_DESKTOP)
      chronoLogMutex.unlock();
    #endif
  }
#endif // CHRONOLOG_THREAD_SAFE

const char* ChronoLogger::getCurrentTaskName() {
  const char* name = NULL;
  #if defined(CHRONOLOG_UNO_Q)
    k_tid_t currentId = k_sched_current_thread_query(); 
    for (int i = 0; i < threadsMapCount; i++) {
      if (threadsMap[i].id == currentId) {
        name = threadsMap[i].name;
        break;
      }
    }
  #elif defined(CHRONOLOG_PLATFORM_DESKTOP)
    static thread_local char threadName[32];
    snprintf(threadName, sizeof(threadName), "Thread-%zu",
      std::hash<std::thread::id>{}(std::this_thread::get_id()) % 1000
    );
    return threadName;
  #elif defined(CHRONOLOG_PLATFORM_ZEPHYR)
      name = k_thread_name_get(k_current_get());
  #elif defined(CHRONOLOG_PLATFORM_STM32_HAL) && defined(CHRONOLOG_STM32_FREERTOS)
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
      name = pcTaskGetName(NULL);
    }
  #elif defined(CHRONOLOG_PLATFORM_ESP_IDF) || (defined(CHRONOLOG_PLATFORM_ARDUINO) && defined(CHRONOLOG_ESP))
    return pcTaskGetName(NULL);
  #endif
  return (name && name[0]) ? name : CHRONOLOG_FALLBACK_TASK_NAME;
}

void ChronoLogger::info(const char* fmt, ...) const { 
  if (chronoLogLevel >= CHRONOLOG_LEVEL_INFO) {
    va_list args;
    va_start(args, fmt);
    print("INFO", CHRONOLOG_COLOR_INFO, fmt, args);
    va_end(args);
  }
}

void ChronoLogger::warn(const char* fmt, ...) const { 
  if (chronoLogLevel >= CHRONOLOG_LEVEL_WARN) {
    va_list args;
    va_start(args, fmt);
    print("WARN", CHRONOLOG_COLOR_WARN, fmt, args);
    va_end(args);
  }
}

void ChronoLogger::debug(const char* fmt, ...) const { 
  if (chronoLogLevel >= CHRONOLOG_LEVEL_DEBUG) { 
    va_list args;
    va_start(args, fmt);
    print("DEBUG", CHRONOLOG_COLOR_DEBUG, fmt, args);
    va_end(args);
  }
}

void ChronoLogger::error(const char* fmt, ...) const { 
  if (chronoLogLevel >= CHRONOLOG_LEVEL_ERROR) {
    va_list args;
    va_start(args, fmt);
    print("ERROR", CHRONOLOG_COLOR_ERROR, fmt, args);
    va_end(args);
  }
}

void ChronoLogger::fatal(const char* fmt, ...) const { 
  if (chronoLogLevel >= CHRONOLOG_LEVEL_FATAL) {
    va_list args;
    va_start(args, fmt);
    print("FATAL", CHRONOLOG_COLOR_FATAL, fmt, args);
    va_end(args);
  }
}

void ChronoLogger::getTimeStamp(char* buffer, size_t len) const {
  #if defined(CHRONOLOG_PLATFORM_ESP_IDF) || (defined(CHRONOLOG_PLATFORM_ARDUINO) && defined(CHRONOLOG_ESP))
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm timeinfo;
    localtime_r(&tv.tv_sec, &timeinfo);
    snprintf(buffer, len, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  #elif defined(CHRONOLOG_PLATFORM_DESKTOP)
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    snprintf(buffer, len, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
  #elif defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_UNO_Q)
    uint64_t ms = k_uptime_get();
    snprintf(buffer, len, "%02llu:%02llu:%02llu",
      (ms / 3600000) % 24, (ms / 60000) % 60, (ms / 1000) % 60);
  #elif defined(CHRONOLOG_PLATFORM_STM32_HAL)
    uint32_t ms = HAL_GetTick();
    snprintf(buffer, len, "%02lu:%02lu:%02lu",
      (ms / 3600000) % 24, (ms / 60000) % 60, (ms / 1000) % 60);
  #endif
}

void ChronoLogger::printInfo(const char* levelStr, const char* color,
                             const char* time_buf, const char* taskName) const {
  #if defined(CHRONOLOG_PLATFORM_STM32_HAL)
    char line_buf[96];
    snprintf(
      line_buf, sizeof(line_buf),
      "%s | %-15s | %s%-8s%s | %-16s | ",
      time_buf, name, color, levelStr, CHRONOLOG_COLOR_RESET, taskName
    );
    if (!uartHandler) return;
    HAL_UART_Transmit(
      uartHandler, (uint8_t*)line_buf, strlen(line_buf), HAL_MAX_DELAY
    );
  #elif defined(CHRONOLOG_PLATFORM_ARDUINO) && defined(CHRONOLOG_ESP)
    Serial.printf(
      "%s | %-15s | %s%-8s%s | %-16s | ",
      time_buf, name, color, levelStr, CHRONOLOG_COLOR_RESET, taskName
    );
  #elif defined(CHRONOLOG_PLATFORM_ARDUINO) && defined(CHRONOLOG_UNO_Q)
    char line_buf[96];
    snprintf(
      line_buf, sizeof(line_buf),
      "%s | %-15s | %s%-8s%s | %-16s | ",
      time_buf, name, color, levelStr, CHRONOLOG_COLOR_RESET, taskName
    );
    Monitor.print(line_buf);
  #elif defined(CHRONOLOG_PLATFORM_ARDUINO) && !defined(CHRONOLOG_ESP)
    char line_buf[96];
    snprintf(
      line_buf, sizeof(line_buf),
      "%s | %-15s | %s%-8s%s | %-16s | ",
      time_buf, name, color, levelStr, CHRONOLOG_COLOR_RESET, taskName
    );
    Serial.print(line_buf);
  #elif defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_PLATFORM_DESKTOP)
    printf(
      "%s | %-15s | %s%-8s%s | %-16s | ",
      time_buf, name, color, levelStr, CHRONOLOG_COLOR_RESET, taskName
    );
  #endif
}

#if CHRONOLOG_PRO_FEATURES
  void ChronoLogger::progress(const uint32_t current, const uint32_t total, const char* title) const {
    if (chronoLogLevel >= CHRONOLOG_LEVEL_PRO_FEATURES) {
      uint32_t  tempCurrent = (current > total) ? total : current;
      uint32_t  tempTotal   = (total == 0) ? 1 : total; // Prevent division by zero
      #if CHRONOLOG_THREAD_SAFE
        threadSafeLock();
      #endif
      if(current < total) { 
        printProgress("PROGRESS", CHRONOLOG_COLOR_PROGS, tempCurrent, tempTotal, title);
      } else { 
        printProgress("PROGRESS", CHRONOLOG_COLOR_PROGF, tempCurrent, tempTotal, title);
        #if defined(CHRONOLOG_PLATFORM_ARDUINO) && defined(CHRONOLOG_UNO_Q)
          Monitor.println();
        #elif defined(CHRONOLOG_PLATFORM_ARDUINO) && !defined(CHRONOLOG_UNO_Q)
          Serial.println();
        #elif defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_PLATFORM_DESKTOP)
          printf("\n");
        #elif defined(CHRONOLOG_PLATFORM_STM32_HAL)
          if (uartHandler) {
            const char* newline = "\n";
            HAL_UART_Transmit(uartHandler, (uint8_t*)newline, strlen(newline), HAL_MAX_DELAY);
          }
        #endif
      }
      #if CHRONOLOG_THREAD_SAFE
        threadSafeUnlock();
      #endif
    }
  }
#endif // CHRONOLOG_PRO_FEATURES

void ChronoLogger::print(const char* levelStr, const char* color, const char* fmt, va_list args) const {
  char time_buf[16];
  getTimeStamp(time_buf, 16);
  const char* taskName = getCurrentTaskName();

  #if CHRONOLOG_THREAD_SAFE
    threadSafeLock();
  #endif

  printInfo(levelStr, color, time_buf, taskName);
  char msg_buf[CHRONOLOG_BUFFER_LEN];
  va_list args_copy;
  va_copy(args_copy, args);
  int len = vsnprintf(msg_buf, sizeof(msg_buf), fmt, args_copy);
  va_end(args_copy);

  if (len < 0) {
    strncpy(msg_buf, "[Format error]", sizeof(msg_buf));
    msg_buf[sizeof(msg_buf) - 1] = '\0';
  } else if (len >= (int)sizeof(msg_buf)) {
    const char trunc_marker[] = "...";
    size_t marker_len = sizeof(trunc_marker) - 1;
    if (sizeof(msg_buf) > marker_len + 1) {
      size_t copy_len = sizeof(msg_buf) - marker_len - 1; // leave room for marker + NUL
      msg_buf[copy_len] = '\0';
      memcpy(&msg_buf[copy_len], trunc_marker, marker_len);
      msg_buf[copy_len + marker_len] = '\0';
    } else {
      msg_buf[sizeof(msg_buf) - 1] = '\0';
    }
  }

  // Common platform-specific output using the (possibly truncated) buffer
  #if defined(CHRONOLOG_PLATFORM_STM32_HAL)
    if (!uartHandler) {
      #if CHRONOLOG_THREAD_SAFE
        threadSafeUnlock();
      #endif
      return;
    }
    HAL_UART_Transmit(uartHandler, (uint8_t*)msg_buf, strlen(msg_buf), HAL_MAX_DELAY);
  #elif defined(CHRONOLOG_PLATFORM_ARDUINO) && defined(CHRONOLOG_UNO_Q)
    Monitor.print(msg_buf);
  #elif defined(CHRONOLOG_PLATFORM_ARDUINO) && !defined(CHRONOLOG_UNO_Q)
    Serial.print(msg_buf);
  #elif defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_PLATFORM_ESP_IDF)  || defined(CHRONOLOG_PLATFORM_DESKTOP)
    printf("%s", msg_buf);
  #endif

  #if defined(CHRONOLOG_PLATFORM_STM32_HAL)
    if (!uartHandler) return;
    const char* newline = "\n\r";
    HAL_UART_Transmit(uartHandler, (uint8_t*)newline, strlen(newline), HAL_MAX_DELAY);
  #elif defined(CHRONOLOG_PLATFORM_ARDUINO) && defined(CHRONOLOG_UNO_Q)
    Monitor.println();
  #elif defined(CHRONOLOG_PLATFORM_ARDUINO) && !defined(CHRONOLOG_UNO_Q)
    Serial.println();
  #elif defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_PLATFORM_DESKTOP)
    printf("\n");
  #endif

  #if CHRONOLOG_REMOTE_ENABLE
    char full_buf[CHRONOLOG_BUFFER_LEN * 2];
    snprintf(
      full_buf, sizeof(full_buf), "%s | %-15s | %s%-8s%s | %-16s | %s\n",
      time_buf, name, color, levelStr, CHRONOLOG_COLOR_RESET, taskName, msg_buf
    );
    ChronoLogRemote::getInstance()->write(full_buf);
  #endif

  #if CHRONOLOG_THREAD_SAFE
    threadSafeUnlock();
  #endif
}

void ChronoLogger::printProgress(const char* levelStr, const char* color, uint32_t current, uint32_t total, const char* title) const {
  char time_buf[16];
  getTimeStamp(time_buf, 16);
  const char* taskName = getCurrentTaskName();
  printInfo(levelStr, color, time_buf, taskName);
  uint8_t percent = (current * 100) / total;
  char prog_buf[100];

  const uint8_t bar_width = 20;
  uint8_t filled_chars = (percent * bar_width) / 100;

  snprintf(prog_buf, sizeof(prog_buf), "%s: %3u%% (%u/%u) [", title, percent, current, total);

  for (uint8_t i = 0; i < bar_width; i++) {
    if (i < filled_chars) {
      strncat(prog_buf, "=", sizeof(prog_buf) - strlen(prog_buf) - 1);
    } else {
      strncat(prog_buf, "-", sizeof(prog_buf) - strlen(prog_buf) - 1);
    }
  }
  strncat(prog_buf, "]", sizeof(prog_buf) - strlen(prog_buf) - 1);

  // Output the progress bar
  #if defined(CHRONOLOG_PLATFORM_STM32_HAL)
    if (!uartHandler) return;
    HAL_UART_Transmit(uartHandler, (uint8_t*)prog_buf, strlen(prog_buf), HAL_MAX_DELAY);
    const char* carriage_return = "\r";
    HAL_UART_Transmit(uartHandler, (uint8_t*)carriage_return, strlen(carriage_return), HAL_MAX_DELAY);
  #elif defined(CHRONOLOG_PLATFORM_ARDUINO) && defined(CHRONOLOG_UNO_Q)
    Monitor.print(prog_buf);
    Monitor.print("\r");  // Carriage return to overwrite same line
    Monitor.flush();      // Ensure output is sent immediately
  #elif defined(CHRONOLOG_PLATFORM_ARDUINO) && !defined(CHRONOLOG_UNO_Q)
    Serial.print(prog_buf);
    Serial.print("\r");  // Carriage return to overwrite same line
    Serial.flush();      // Ensure output is sent immediately
  #elif defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_PLATFORM_DESKTOP)
    printf("%s\r", prog_buf);  // Carriage return to overwrite same line
    fflush(stdout);            // Ensure output is sent immediately
  #endif
}

#endif // CHRONOLOG_MODE