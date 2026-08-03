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

#if CHRONOLOG_PRO_FEATURES
  struct PlotSeries {
    char name[16];
    float buf[CHRONOLOG_PLOT_WINDOW];
    uint32_t count;          // Total samples pushed (ring index = count % CHRONOLOG_PLOT_WINDOW)
    bool initialized;
  };
  static PlotSeries plotSeries[CHRONOLOG_PLOT_SERIES];
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
    if (!uartHandler) return;
    char line_buf[96];
    snprintf(
      line_buf, sizeof(line_buf),
      "%s | %-15s | %s%-8s%s | %-16s | ",
      time_buf, name, color, levelStr, CHRONOLOG_COLOR_RESET, taskName
    );
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

  // ---- Graph Plotter -------------------------------------------------------

  void ChronoLogger::plot(const char* series, float value) const {
    if (chronoLogLevel < CHRONOLOG_LEVEL_PRO_FEATURES || series == NULL) return;
    #if CHRONOLOG_THREAD_SAFE
      threadSafeLock();
    #endif
    PlotSeries* s = findPlotSeries(series);
    if (s != NULL) {
      s->buf[s->count % CHRONOLOG_PLOT_WINDOW] = value;
      s->count++;
      renderSparkline(series);
    }
    #if CHRONOLOG_THREAD_SAFE
      threadSafeUnlock();
    #endif
  }

  void ChronoLogger::plot(const char* series, const float* values, size_t count) const {
    if (chronoLogLevel < CHRONOLOG_LEVEL_PRO_FEATURES || series == NULL || values == NULL || count == 0) return;
    #if CHRONOLOG_THREAD_SAFE
      threadSafeLock();
    #endif
    PlotSeries* s = findPlotSeries(series);
    if (s != NULL) {
      for (size_t i = 0; i < count; i++) {
        s->buf[s->count % CHRONOLOG_PLOT_WINDOW] = values[i];
        s->count++;
      }
      renderWindowChart(series);
    }
    #if CHRONOLOG_THREAD_SAFE
      threadSafeUnlock();
    #endif
  }

  void ChronoLogger::plotWindow(const char* series) const {
    if (chronoLogLevel < CHRONOLOG_LEVEL_PRO_FEATURES || series == NULL) return;
    #if CHRONOLOG_THREAD_SAFE
      threadSafeLock();
    #endif
    renderWindowChart(series);
    #if CHRONOLOG_THREAD_SAFE
      threadSafeUnlock();
    #endif
  }

  void ChronoLogger::plotWindow() const {
    if (chronoLogLevel < CHRONOLOG_LEVEL_PRO_FEATURES) return;
    #if CHRONOLOG_THREAD_SAFE
      threadSafeLock();
    #endif
    for (int i = 0; i < CHRONOLOG_PLOT_SERIES; i++) {
      if (plotSeries[i].initialized) {
        renderWindowChart(plotSeries[i].name);
      }
    }
    #if CHRONOLOG_THREAD_SAFE
      threadSafeUnlock();
    #endif
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
    // Lock order: chronoLogMutex (already held) -> clientsMutex (inside write()).
    // ChronoLogRemote never calls back into ChronoLogger, so this is safe.
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

#if CHRONOLOG_PRO_FEATURES
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

  // ---- Graph plotter helpers ------------------------------------------------

  PlotSeries* ChronoLogger::findPlotSeries(const char* series) const {
    for (int i = 0; i < CHRONOLOG_PLOT_SERIES; i++) {
      if (plotSeries[i].initialized && strcmp(plotSeries[i].name, series) == 0) {
        return &plotSeries[i];
      }
    }
    for (int i = 0; i < CHRONOLOG_PLOT_SERIES; i++) {
      if (!plotSeries[i].initialized) {
        strncpy(plotSeries[i].name, series, sizeof(plotSeries[i].name) - 1);
        plotSeries[i].name[sizeof(plotSeries[i].name) - 1] = '\0';
        plotSeries[i].count = 0;
        plotSeries[i].initialized = true;
        return &plotSeries[i];
      }
    }
    return NULL;  // Registry full
  }

  void ChronoLogger::renderSparkline(const char* series) const {
    PlotSeries* s = findPlotSeries(series);
    if (s == NULL || s->count == 0) return;
    uint32_t span = (s->count < CHRONOLOG_PLOT_WINDOW) ? s->count : CHRONOLOG_PLOT_WINDOW;
    uint32_t start = s->count - span;

    float min = s->buf[start % CHRONOLOG_PLOT_WINDOW];
    float max = min;
    for (uint32_t i = 1; i < span; i++) {
      float v = s->buf[(start + i) % CHRONOLOG_PLOT_WINDOW];
      if (v < min) min = v;
      if (v > max) max = v;
    }
    float range = max - min;
    if (range == 0.0f) range = 1.0f;  // Flat series: mid glyph

    char line_buf[4 + CHRONOLOG_PLOT_WINDOW * 4 + 64];  // name + glyphs (3B each + space) + stats
    int pos = 0;
    pos += snprintf(line_buf + pos, sizeof(line_buf) - pos, "%s |", series);
    for (uint32_t i = 0; i < span; i++) {
      float v = s->buf[(start + i) % CHRONOLOG_PLOT_WINDOW];
      int idx = (int)((v - min) / range * 7.0f);
      if (idx < 0) idx = 0;
      if (idx > 7) idx = 7;
      pos += snprintf(line_buf + pos, sizeof(line_buf) - pos, " %s", CHRONOLOG_PLOT_BLOCKS[idx]);
      if ((size_t)pos >= sizeof(line_buf)) pos = (int)sizeof(line_buf) - 1;  // Never point past the buffer
    }
    pos += snprintf(line_buf + pos, sizeof(line_buf) - pos, " | min=%.1f max=%.1f last=%.1f",
                    min, max, s->buf[(s->count - 1) % CHRONOLOG_PLOT_WINDOW]);
    if ((size_t)pos >= sizeof(line_buf)) pos = (int)sizeof(line_buf) - 1;
    snprintf(line_buf + pos, sizeof(line_buf) - pos, "\r");

    #if defined(CHRONOLOG_PLATFORM_STM32_HAL)
      if (uartHandler) HAL_UART_Transmit(uartHandler, (uint8_t*)line_buf, strlen(line_buf), HAL_MAX_DELAY);
    #elif defined(CHRONOLOG_PLATFORM_ARDUINO) && defined(CHRONOLOG_UNO_Q)
      Monitor.print(line_buf);
      Monitor.flush();
    #elif defined(CHRONOLOG_PLATFORM_ARDUINO) && !defined(CHRONOLOG_UNO_Q)
      Serial.print(line_buf);
      Serial.flush();
    #elif defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_PLATFORM_DESKTOP)
      printf("%s", line_buf);
      fflush(stdout);
    #endif
  }

  void ChronoLogger::renderWindowChart(const char* series) const {
    PlotSeries* s = findPlotSeries(series);
    if (s == NULL || s->count == 0) return;
    uint32_t span = (s->count < CHRONOLOG_PLOT_WINDOW) ? s->count : CHRONOLOG_PLOT_WINDOW;
    uint32_t start = s->count - span;

    float min = s->buf[start % CHRONOLOG_PLOT_WINDOW];
    float max = min;
    for (uint32_t i = 1; i < span; i++) {
      float v = s->buf[(start + i) % CHRONOLOG_PLOT_WINDOW];
      if (v < min) min = v;
      if (v > max) max = v;
    }
    float range = max - min;
    if (range == 0.0f) range = 1.0f;  // Flat series: mid row

    char line_buf[64 + CHRONOLOG_PLOT_WINDOW * 4];
    snprintf(line_buf, sizeof(line_buf), "%s  window=%u  min=%.1f  max=%.1f  last=%.1f\n",
             series, (unsigned)span, min, max, s->buf[(s->count - 1) % CHRONOLOG_PLOT_WINDOW]);
    outputPlotLine(line_buf);

    for (int row = CHRONOLOG_PLOT_ROWS - 1; row >= 0; row--) {
      int pos = 0;
      pos += snprintf(line_buf + pos, sizeof(line_buf) - pos, "%2d |", row);
      for (uint32_t i = 0; i < span; i++) {
        float v = s->buf[(start + i) % CHRONOLOG_PLOT_WINDOW];
        int cell = (int)((v - min) / range * (CHRONOLOG_PLOT_ROWS - 1) + 0.5f);
        if (cell < 0) cell = 0;
        if (cell > CHRONOLOG_PLOT_ROWS - 1) cell = CHRONOLOG_PLOT_ROWS - 1;
        pos += snprintf(line_buf + pos, sizeof(line_buf) - pos, " %s",
                        (cell >= row) ? CHRONOLOG_PLOT_BLOCKS[7] : " ");
        if ((size_t)pos >= sizeof(line_buf)) pos = (int)sizeof(line_buf) - 1;  // Never point past the buffer
      }
      pos += snprintf(line_buf + pos, sizeof(line_buf) - pos, "\n");
      outputPlotLine(line_buf);
    }
  }

  void ChronoLogger::outputPlotLine(const char* line) const {
    #if defined(CHRONOLOG_PLATFORM_STM32_HAL)
      if (uartHandler) HAL_UART_Transmit(uartHandler, (uint8_t*)line, strlen(line), HAL_MAX_DELAY);
    #elif defined(CHRONOLOG_PLATFORM_ARDUINO) && defined(CHRONOLOG_UNO_Q)
      Monitor.print(line);
      Monitor.flush();
    #elif defined(CHRONOLOG_PLATFORM_ARDUINO) && !defined(CHRONOLOG_UNO_Q)
      Serial.print(line);
      Serial.flush();
    #elif defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_PLATFORM_DESKTOP)
      printf("%s", line);
      fflush(stdout);
    #endif
  }

#endif // CHRONOLOG_PRO_FEATURES

#endif // CHRONOLOG_MODE