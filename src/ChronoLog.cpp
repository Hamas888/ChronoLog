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
    uint32_t t_ms[CHRONOLOG_PLOT_WINDOW];  // Uptime timestamp per sample (same ring index as buf)
    uint32_t count;          // Total samples pushed (ring index = count % CHRONOLOG_PLOT_WINDOW)
    uint32_t tWindowMs;      // Time-window (ms) this series is bucketed into (0 = per-sample)
    bool initialized;
  };
  static PlotSeries plotSeries[CHRONOLOG_PLOT_SERIES];

  // ANSI live chart state: number of lines of the chart currently on screen.
  // Non-zero means a live chart was drawn and needs cursor-up + erase before redraw.
  static int liveChartLines = 0;

  // Plot glyphs, low to high. Override CHRONOLOG_PLOT_BLOCKS with an 8-element
  // initializer for ASCII-only terminals, e.g. { " ", ".", ":", "-", "=", "+", "*", "#" }.
  #ifndef CHRONOLOG_PLOT_BLOCKS
    #define CHRONOLOG_PLOT_BLOCKS                     { "\xE2\x96\x81", "\xE2\x96\x82", "\xE2\x96\x83", "\xE2\x96\x84", "\xE2\x96\x85", "\xE2\x96\x86", "\xE2\x96\x87", "\xE2\x96\x88" }  // ▁▂▃▄▅▆▇█
  #endif
  static const char* const plotBlocks[8] = CHRONOLOG_PLOT_BLOCKS;
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

uint32_t ChronoLogger::getUptimeMs() const {
  #if defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_UNO_Q)
    return (uint32_t)k_uptime_get();
  #elif defined(CHRONOLOG_PLATFORM_STM32_HAL)
    return (uint32_t)HAL_GetTick();
  #elif defined(CHRONOLOG_PLATFORM_ESP_IDF)
    return (uint32_t)(esp_timer_get_time() / 1000u);
  #elif (defined(CHRONOLOG_PLATFORM_ARDUINO) && defined(CHRONOLOG_ESP))
    return (uint32_t)millis();
  #elif defined(CHRONOLOG_PLATFORM_DESKTOP)
    // Millisecond clock since an arbitrary origin (steady, not wall time).
    return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch()).count();
  #else
    return 0;  // No monotonic source -> timeWindowSec ignored, per-sample rendering
  #endif
}

void ChronoLogger::formatElapsed(char* buffer, size_t len, uint32_t elapsedSec) const {
  uint32_t h = elapsedSec / 3600u;
  uint32_t m = (elapsedSec % 3600u) / 60u;
  uint32_t s = elapsedSec % 60u;
  snprintf(buffer, len, "%02u:%02u:%02u", (unsigned)h, (unsigned)m, (unsigned)s);
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

  void ChronoLogger::plot(const char* series, float value, uint32_t timeWindowSec) const {
    if (chronoLogLevel < CHRONOLOG_LEVEL_PRO_FEATURES || series == NULL) return;
    #if CHRONOLOG_THREAD_SAFE
      threadSafeLock();
    #endif
    PlotSeries* s = findPlotSeries(series);
    if (s != NULL) {
      s->tWindowMs = timeWindowSec * 1000u;
      uint32_t idx = s->count % CHRONOLOG_PLOT_WINDOW;
      s->buf[idx] = value;
      s->t_ms[idx] = getUptimeMs();
      s->count++;

      #if CHRONOLOG_PLOT_ANSI
        // Live multi-row chart: move cursor to the top of the previous chart
        // (if any), then redraw the full chart in place.
        #if defined(CHRONOLOG_PLATFORM_STM32_HAL)
          if (uartHandler) {
            char esc[24];
            if (liveChartLines > 0) {
              int n = snprintf(esc, sizeof(esc), "\x1b[%dA", liveChartLines);
              HAL_UART_Transmit(uartHandler, (uint8_t*)esc, n, HAL_MAX_DELAY);
            }
            const char* hide = "\x1b[?25l";
            HAL_UART_Transmit(uartHandler, (uint8_t*)hide, strlen(hide), HAL_MAX_DELAY);
          }
        #elif defined(CHRONOLOG_PLATFORM_DESKTOP) || defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_PLATFORM_ESP_IDF) || \
              (defined(CHRONOLOG_PLATFORM_ARDUINO) && !defined(CHRONOLOG_UNO_Q))
          if (liveChartLines > 0) {
            printf("\x1b[%dA", liveChartLines);
          }
          printf("\x1b[?25l");   // Hide cursor during redraw
          fflush(stdout);
        #endif

        renderWindowChart(series);

        #if defined(CHRONOLOG_PLATFORM_STM32_HAL)
          if (uartHandler) {
            const char* show = "\x1b[?25h";
            HAL_UART_Transmit(uartHandler, (uint8_t*)show, strlen(show), HAL_MAX_DELAY);
          }
        #elif defined(CHRONOLOG_PLATFORM_DESKTOP) || defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_PLATFORM_ESP_IDF) || \
              (defined(CHRONOLOG_PLATFORM_ARDUINO) && !defined(CHRONOLOG_UNO_Q))
          printf("\x1b[?25h");   // Restore cursor
          fflush(stdout);
        #endif
        liveChartLines = CHRONOLOG_PLOT_ROWS + 2;   // header + rows + time line
      #else
        // Single-line sparkline fallback (bare UART / plain serial monitor).
        renderSparkline(series, timeWindowSec);
      #endif // CHRONOLOG_PLOT_ANSI
    }
    #if CHRONOLOG_THREAD_SAFE
      threadSafeUnlock();
    #endif
  }

  void ChronoLogger::plot(const char* series, const float* values, size_t count, uint32_t timeWindowSec) const {
    if (chronoLogLevel < CHRONOLOG_LEVEL_PRO_FEATURES || series == NULL || values == NULL || count == 0) return;
    #if CHRONOLOG_THREAD_SAFE
      threadSafeLock();
    #endif
    PlotSeries* s = findPlotSeries(series);
    if (s != NULL) {
      s->tWindowMs = timeWindowSec * 1000u;
      uint32_t now = getUptimeMs();
      for (size_t i = 0; i < count; i++) {
        s->buf[s->count % CHRONOLOG_PLOT_WINDOW] = values[i];
        s->t_ms[s->count % CHRONOLOG_PLOT_WINDOW] = now;
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
        plotSeries[i].tWindowMs = 0;
        plotSeries[i].initialized = true;
        return &plotSeries[i];
      }
    }
    return NULL;  // Registry full
  }

  // Returns the number of columns to render for the given span.
  // timeWindowSec == 0 -> per-sample (span columns).
  // Otherwise -> timeWindowSec columns (bucketed), capped at span.
  uint32_t ChronoLogger::renderBucketed(const PlotSeries* s, uint32_t span, uint32_t start,
                                        uint32_t timeWindowSec, float* outMin, float* outMax) const {
    uint32_t cols = span;
    uint32_t windowMs = timeWindowSec * 1000u;
    if (windowMs > 0) {
      cols = timeWindowSec;
      if (cols == 0) cols = 1;
      if (cols > span) cols = span;
    }
    // Compute min/max over the actual rendered values.
    float mn = 0.0f, mx = 0.0f;
    for (uint32_t i = 0; i < cols; i++) {
      float v = s->buf[(start + i) % CHRONOLOG_PLOT_WINDOW];
      if (i == 0) { mn = mx = v; }
      else { if (v < mn) mn = v; if (v > mx) mx = v; }
    }
    *outMin = mn; *outMax = mx;
    return cols;
  }

  void ChronoLogger::renderSparkline(const char* series, uint32_t timeWindowSec) const {
    PlotSeries* s = findPlotSeries(series);
    if (s == NULL || s->count == 0) return;
    uint32_t span = (s->count < CHRONOLOG_PLOT_WINDOW) ? s->count : CHRONOLOG_PLOT_WINDOW;
    uint32_t start = s->count - span;

    float min = 0.0f, max = 0.0f;
    uint32_t cols = renderBucketed(s, span, start, timeWindowSec, &min, &max);
    float range = max - min;
    if (range == 0.0f) range = 1.0f;  // Flat series: mid glyph

    char line_buf[4 + CHRONOLOG_PLOT_WINDOW * 4 + 128];  // name + left/right labels + glyphs + time
    int pos = 0;
    pos += snprintf(line_buf + pos, sizeof(line_buf) - pos, "%s | min %.1f |", series, min);
    if ((size_t)pos >= sizeof(line_buf)) pos = (int)sizeof(line_buf) - 1;
    for (uint32_t i = 0; i < cols; i++) {
      float v = s->buf[(start + i) % CHRONOLOG_PLOT_WINDOW];
      int idx = (int)((v - min) / range * 7.0f);
      if (idx < 0) idx = 0;
      if (idx > 7) idx = 7;
      pos += snprintf(line_buf + pos, sizeof(line_buf) - pos, " %s", plotBlocks[idx]);
      if ((size_t)pos >= sizeof(line_buf)) pos = (int)sizeof(line_buf) - 1;
    }
    char tbuf[16];
    if (cols >= 2 && s->t_ms[start % CHRONOLOG_PLOT_WINDOW] &&
        s->t_ms[(start + cols - 1) % CHRONOLOG_PLOT_WINDOW]) {
      uint32_t t1 = s->t_ms[(start + cols - 1) % CHRONOLOG_PLOT_WINDOW];
      uint32_t t0 = s->t_ms[start % CHRONOLOG_PLOT_WINDOW];
      formatElapsed(tbuf, sizeof(tbuf), (t1 > t0) ? (t1 - t0) / 1000u : 0);
    } else {
      snprintf(tbuf, sizeof(tbuf), "00:00:00");
    }
    pos += snprintf(line_buf + pos, sizeof(line_buf) - pos, " | max %.1f | last %.1f | t %s\r",
                    max, s->buf[(s->count - 1) % CHRONOLOG_PLOT_WINDOW], tbuf);
    if ((size_t)pos >= sizeof(line_buf)) pos = (int)sizeof(line_buf) - 1;

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

    float min = 0.0f, max = 0.0f;
    uint32_t cols = renderBucketed(s, span, start, s->tWindowMs / 1000u, &min, &max);
    float range = max - min;
    if (range == 0.0f) range = 1.0f;  // Flat series: mid row

    char line_buf[64 + CHRONOLOG_PLOT_WINDOW * 4 + 128];
    snprintf(line_buf, sizeof(line_buf), "%s  window=%u  min=%.1f  max=%.1f  last=%.1f\n",
             series, (unsigned)cols, min, max, s->buf[(s->count - 1) % CHRONOLOG_PLOT_WINDOW]);
    outputPlotLine(line_buf);

    for (int row = CHRONOLOG_PLOT_ROWS - 1; row >= 0; row--) {
      int pos = 0;
      pos += snprintf(line_buf + pos, sizeof(line_buf) - pos, "%2d |", row);
      for (uint32_t i = 0; i < cols; i++) {
        float v = s->buf[(start + i) % CHRONOLOG_PLOT_WINDOW];
        int cell = (int)((v - min) / range * (CHRONOLOG_PLOT_ROWS - 1) + 0.5f);
        if (cell < 0) cell = 0;
        if (cell > CHRONOLOG_PLOT_ROWS - 1) cell = CHRONOLOG_PLOT_ROWS - 1;
        bool isFull = (cell >= row);
        pos += snprintf(line_buf + pos, sizeof(line_buf) - pos, " %s", isFull ? plotBlocks[7] : " ");
        if ((size_t)pos >= sizeof(line_buf)) pos = (int)sizeof(line_buf) - 1;
      }
      if (row == CHRONOLOG_PLOT_ROWS - 1) {
        pos += snprintf(line_buf + pos, sizeof(line_buf) - pos, " | max %.1f\n", max);
      } else if (row == 0) {
        pos += snprintf(line_buf + pos, sizeof(line_buf) - pos, " | min %.1f\n", min);
      } else {
        pos += snprintf(line_buf + pos, sizeof(line_buf) - pos, "\n");
      }
      outputPlotLine(line_buf);
    }

    // X-axis time range line: t start to end (elapsed across the rendered window).
    if (cols >= 2 && s->t_ms[start % CHRONOLOG_PLOT_WINDOW] &&
        s->t_ms[(start + cols - 1) % CHRONOLOG_PLOT_WINDOW]) {
      uint32_t t1 = s->t_ms[(start + cols - 1) % CHRONOLOG_PLOT_WINDOW];
      uint32_t t0 = s->t_ms[start % CHRONOLOG_PLOT_WINDOW];
      char t0s[16], t1s[16];
      formatElapsed(t0s, sizeof(t0s), t0 / 1000u);
      formatElapsed(t1s, sizeof(t1s), t1 / 1000u);
      snprintf(line_buf, sizeof(line_buf), "t %s to %s\n", t0s, t1s);
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