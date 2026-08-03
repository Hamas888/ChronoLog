/*
 ============================================================================================================================================
 * File:        ChronoLog.h
 * Author:      Hamas Saeed
 * Version:     Rev_1.1.5
 * Date:        Dec 26 2025
 * Brief:       This file provides Debuging / Logging functionalities for embedded & Desktop systems (Arduino, ESP-IDF, Zephyr, STM32 HAL).
 ============================================================================================================================================
 * License: 
 * MIT License
 * 
 * Copyright (c) 2025 Hamas Saeed
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, 
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do 
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * For any inquiries, contact Hamas Saeed at hamasaeed@gmail.com
 ============================================================================================================================================
 */

#ifndef CHRONOLOG_H
#define CHRONOLOG_H

#if defined(ARDUINO)                                                                                               // Platform detection
  #include <Arduino.h>
  #if defined(ESP32)
    #define CHRONOLOG_ESP
    #include <freertos/task.h>
    #include <freertos/FreeRTOS.h>
  #elif defined(ARDUINO_UNO_Q)
    #define CHRONOLOG_UNO_Q
    #include <time.h>
    #include <zephyr/kernel.h>
    #include <Arduino_RouterBridge.h>
  #endif
  #define CHRONOLOG_PLATFORM_ARDUINO
#elif defined(ESP_PLATFORM)
  #include <time.h>
  #include <esp_log.h>
  #include <sys/time.h>
  #include <freertos/task.h>
  #include <freertos/FreeRTOS.h>
  #define CHRONOLOG_PLATFORM_ESP_IDF
#elif defined(__ZEPHYR__)
    #include <time.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdarg.h>
    #include <string.h>
    #include <zephyr/kernel.h>
    #include <zephyr/sys/printk.h>
  #define CHRONOLOG_PLATFORM_ZEPHYR
#elif defined(__arm__) && (defined(USE_HAL_DRIVER) || defined(USE_FULL_LL_DRIVER))
  #include "main.h"
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdarg.h>
  #include <string.h>
  #if defined(osCMSIS) || defined(FREERTOS)
    #define CHRONOLOG_STM32_FREERTOS
    #include "cmsis_os.h"
  #endif
  #define CHRONOLOG_PLATFORM_STM32_HAL
#elif defined(STM32F0) || defined(STM32F1) || defined(STM32F3) || defined(STM32F4) || defined(STM32F7) || \
      defined(STM32G0) || defined(STM32G4) || defined(STM32H7) || defined(STM32L0) || defined(STM32L1) || \
      defined(STM32L4) || defined(STM32L5) || defined(STM32WB) || defined(STM32WL)
  #include "main.h"
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdarg.h>
  #include <string.h>
  #if defined(osCMSIS) || defined(FREERTOS)
    #define CHRONOLOG_STM32_FREERTOS
    #include "cmsis_os.h"
  #endif
  #define CHRONOLOG_PLATFORM_STM32_HAL
#elif defined(__linux__) || defined(_WIN32) || defined(__APPLE__)
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdarg.h>
  #include <string.h>
  #include <time.h>
  #include <mutex>
  #include <thread>
  #define CHRONOLOG_PLATFORM_DESKTOP
#endif // Platform detection

#ifndef CHRONOLOG_MODE
  #ifdef CONFIG_CHRONOLOG_MODE
    #define CHRONOLOG_MODE                            CONFIG_CHRONOLOG_MODE
  #else
    #define CHRONOLOG_MODE                            1                                                              // Set to 0 to disable logging
  #endif
#endif // CHRONOLOG_MODE

#ifndef CHRONOLOG_THREAD_SAFE
  #ifdef CONFIG_CHRONOLOG_THREAD_SAFE
    #define CHRONOLOG_THREAD_SAFE                     CONFIG_CHRONOLOG_THREAD_SAFE
  #else
    #define CHRONOLOG_THREAD_SAFE                     1                                                              // Set to 0 to disable thread safety (if not using RTOS)
  #endif
#endif // CHRONOLOG_THREAD_SAFE

#ifndef CHRONOLOG_COLOR_ENABLE
  #ifdef CONFIG_CHRONOLOG_COLOR_ENABLE
    #define CHRONOLOG_COLOR_ENABLE                    CONFIG_CHRONOLOG_COLOR_ENABLE
  #else
    #define CHRONOLOG_COLOR_ENABLE                    0                                                              // Set to 0 to disable colored output (if not supported by terminal)
  #endif
#endif // CHRONOLOG_COLOR_ENABLE

#ifndef CHRONOLOG_PRO_FEATURES
  #ifdef CONFIG_CHRONOLOG_PRO_FEATURES
    #define CHRONOLOG_PRO_FEATURES                    CONFIG_CHRONOLOG_PRO_FEATURES
  #else
    #define CHRONOLOG_PRO_FEATURES                    0                                                              // Set to 1 to enable Pro features (e.g. progress start/stop)
  #endif
#endif // CHRONOLOG_PRO_FEATURES

#ifndef CHRONOLOG_REMOTE_ENABLE
  #ifdef CONFIG_CHRONOLOG_REMOTE_ENABLE
    #define CHRONOLOG_REMOTE_ENABLE                   CONFIG_CHRONOLOG_REMOTE_ENABLE
  #else
    #define CHRONOLOG_REMOTE_ENABLE                   0                                                              // Set to 1 to enable remote logging
  #endif
#endif // CHRONOLOG_REMOTE_ENABLE

#ifndef CHRONOLOG_BUFFER_LEN
  #ifdef CONFIG_CHRONOLOG_BUFFER_LEN
    #define CHRONOLOG_BUFFER_LEN                      CONFIG_CHRONOLOG_BUFFER_LEN
  #else
    #define CHRONOLOG_BUFFER_LEN                      256                                                            // Buffer length for formatted messages (increase if needed)
  #endif
#endif // CHRONOLOG_BUFFER_LEN

#ifndef CHRONOLOG_MAX_THREADS
  #ifdef CONFIG_CHRONOLOG_MAX_THREADS
    #define CHRONOLOG_MAX_THREADS                     CONFIG_CHRONOLOG_MAX_THREADS
  #else
    #define CHRONOLOG_MAX_THREADS                     10                                                             // Maximum number of threads to track (for RTOS platforms)
  #endif
#endif // CHRONOLOG_MAX_THREADS

#ifndef CHRONOLOG_DEFAULT_LEVEL
 #ifdef CONFIG_CHRONOLOG_DEFAULT_LEVEL
    #define CHRONOLOG_DEFAULT_LEVEL                    ChronoLogLevel(CONFIG_CHRONOLOG_DEFAULT_LEVEL)
  #else
    #define CHRONOLOG_DEFAULT_LEVEL                    ChronoLogLevel(5)                                               // Default log level (can be overridden per instance)
  #endif
#endif // CHRONOLOG_DEFAULT_LEVEL

#ifndef CHRONOLOG_FALLBACK_TASK_NAME
  #ifdef CONFIG_CHRONOLOG_FALLBACK_TASK_NAME
    #define CHRONOLOG_FALLBACK_TASK_NAME              CONFIG_CHRONOLOG_FALLBACK_TASK_NAME
  #else
    #define CHRONOLOG_FALLBACK_TASK_NAME              "MainTask"                                                      // Fallback task name if RTOS task name is unavailable
  #endif
#endif // CHRONOLOG_FALLBACK_TASK_NAME

#ifndef CHRONOLOG_PLOT_WINDOW
  #ifdef CONFIG_CHRONOLOG_PLOT_WINDOW
    #define CHRONOLOG_PLOT_WINDOW                     CONFIG_CHRONOLOG_PLOT_WINDOW
  #else
    #define CHRONOLOG_PLOT_WINDOW                     64                                                              // Samples retained per plot series (ring buffer)
  #endif
#endif // CHRONOLOG_PLOT_WINDOW

#ifndef CHRONOLOG_PLOT_SERIES
  #ifdef CONFIG_CHRONOLOG_PLOT_SERIES
    #define CHRONOLOG_PLOT_SERIES                     CONFIG_CHRONOLOG_PLOT_SERIES
  #else
    #define CHRONOLOG_PLOT_SERIES                     4                                                               // Maximum number of named plot series tracked simultaneously
  #endif
#endif // CHRONOLOG_PLOT_SERIES

#ifndef CHRONOLOG_PLOT_ROWS
  #ifdef CONFIG_CHRONOLOG_PLOT_ROWS
    #define CHRONOLOG_PLOT_ROWS                       CONFIG_CHRONOLOG_PLOT_ROWS
  #else
    #define CHRONOLOG_PLOT_ROWS                       5                                                               // Chart height (rows) for plotWindow()
  #endif
#endif // CHRONOLOG_PLOT_ROWS

// Live ANSI chart mode: when enabled, plot(series, value) redraws a full multi-row
// chart in place using ANSI cursor-up/erase-line sequences (desktop terminals,
// PuTTY, screen, minicom). When disabled (bare UART / plain serial monitors),
// plot() falls back to a single-line sparkline overwritten with '\r'.
#ifndef CHRONOLOG_PLOT_ANSI
  #ifdef CONFIG_CHRONOLOG_PLOT_ANSI
    #define CHRONOLOG_PLOT_ANSI                        CONFIG_CHRONOLOG_PLOT_ANSI
  #elif defined(CHRONOLOG_PLATFORM_DESKTOP)
    #define CHRONOLOG_PLOT_ANSI                        1                                                               // Desktop terminals: ANSI live chart by default
  #else
    #define CHRONOLOG_PLOT_ANSI                        0                                                               // Bare UART: single-line sparkline
  #endif
#endif // CHRONOLOG_PLOT_ANSI

#if CHRONOLOG_REMOTE_ENABLE
  #include "ChronoLogRemote.h"
#endif // CHRONOLOG_REMOTE_ENABLE

#if defined(CHRONOLOG_UNO_Q)
  #include "ChronoLogUnoQ.h"
#endif // CHRONOLOG_UNO_Q

#if CHRONOLOG_COLOR_ENABLE
  #define CHRONOLOG_COLOR_INFO                    "\033[3m\033[92m"                                               // Italic + Green
  #define CHRONOLOG_COLOR_WARN                    "\033[3m\033[93m"                                               // Italic + Yellow
  #define CHRONOLOG_COLOR_ERROR                   "\033[3m\033[91m"                                               // Italic + Red
  #define CHRONOLOG_COLOR_DEBUG                   "\033[3m\033[94m"                                               // Italic + Blue
  #define CHRONOLOG_COLOR_FATAL                   "\033[3m\033[95m"                                               // Italic + Magenta
  #define CHRONOLOG_COLOR_RESET                   "\033[0m"                                                       // Reset
#if CHRONOLOG_PRO_FEATURES
  #define CHRONOLOG_COLOR_PROGF                   "\033[3m\033[96m"                                               // Italic + Cyan
  #define CHRONOLOG_COLOR_PROGS                   "\033[3m\033[38;5;208m"                                         // Italic + Orange
#endif // CHRONOLOG_PRO_FEATURES
#else
  #define CHRONOLOG_COLOR_INFO                    ""
  #define CHRONOLOG_COLOR_WARN                    ""
  #define CHRONOLOG_COLOR_ERROR                   ""
  #define CHRONOLOG_COLOR_DEBUG                   ""
  #define CHRONOLOG_COLOR_FATAL                   ""
  #define CHRONOLOG_COLOR_RESET                   ""
#if CHRONOLOG_PRO_FEATURES
  #define CHRONOLOG_COLOR_PROGF                   ""
  #define CHRONOLOG_COLOR_PROGS                   ""
#endif // CHRONOLOG_PRO_FEATURES
#endif // CHRONOLOG_COLOR_ENABLE

enum ChronoLogLevel {
  CHRONOLOG_LEVEL_NONE,
  CHRONOLOG_LEVEL_FATAL,
  CHRONOLOG_LEVEL_ERROR,
  CHRONOLOG_LEVEL_WARN,
  CHRONOLOG_LEVEL_INFO,
  CHRONOLOG_LEVEL_DEBUG,
  #if CHRONOLOG_PRO_FEATURES
    CHRONOLOG_LEVEL_PRO_FEATURES
  #endif // CHRONOLOG_PRO_FEATURES
};

#if CHRONOLOG_THREAD_SAFE
  #if defined(CHRONOLOG_PLATFORM_DESKTOP)
    extern std::mutex chronoLogMutex;
  #elif defined(CHRONOLOG_STM32_FREERTOS)
    extern osMutexId_t chronoLogMutex;
  #elif defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_ESP)
    extern SemaphoreHandle_t chronoLogMutex;
  #elif defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_UNO_Q)
    extern struct k_mutex chronoLogMutex;
  #endif
#endif // CHRONOLOG_THREAD_SAFE

#if CHRONOLOG_MODE

#if CHRONOLOG_PRO_FEATURES
  struct PlotSeries;
#endif // CHRONOLOG_PRO_FEATURES

class ChronoLogger {
public:
  ChronoLogger(const char* moduleName, ChronoLogLevel level = CHRONOLOG_DEFAULT_LEVEL);

  #if defined(CHRONOLOG_UNO_Q)
    void setThreadName(const char* name);
  #endif // CHRONOLOG_UNO_Q

  void setLevel(ChronoLogLevel level)                 { chronoLogLevel = level; }
  #if defined(CHRONOLOG_PLATFORM_STM32_HAL)
    void setUartHandler(UART_HandleTypeDef* handler)  { uartHandler = handler;  }
  #endif // CHRONOLOG_PLATFORM_STM32_HAL


  void info(const char* fmt, ...) const;
  void warn(const char* fmt, ...) const;
  void debug(const char* fmt, ...) const;
  void error(const char* fmt, ...) const;
  void fatal(const char* fmt, ...) const;

  #if CHRONOLOG_PRO_FEATURES
    void progress(const uint32_t current, const uint32_t total, const char* title) const;

    // timeWindowSec > 0: time-bucketed X-axis (samples averaged per time slice).
    // timeWindowSec == 0 (default): per-sample X-axis.
    void plot(const char* series, float value, uint32_t timeWindowSec = 0) const;
    void plot(const char* series, const float* values, size_t count, uint32_t timeWindowSec = 0) const;
    void plotWindow(const char* series) const;
    void plotWindow() const;
  #endif // CHRONOLOG_PRO_FEATURES

private:
  #if CHRONOLOG_PRO_FEATURES
    void printProgress(const char* levelStr, const char* color, uint32_t current, uint32_t total, const char* title) const;
    PlotSeries* findPlotSeries(const char* series) const;
    uint32_t renderBucketed(const PlotSeries* s, uint32_t span, uint32_t start,
                            uint32_t timeWindowSec, float* outMin, float* outMax) const;
    void renderSparkline(const char* series, uint32_t timeWindowSec) const;
    void renderWindowChart(const char* series) const;
    void outputPlotLine(const char* line) const;
    uint32_t getUptimeMs() const;
    void formatElapsed(char* buffer, size_t len, uint32_t elapsedSec) const;
  #endif // CHRONOLOG_PRO_FEATURES

  const char* name;
  ChronoLogLevel chronoLogLevel;

  #if defined(CHRONOLOG_PLATFORM_STM32_HAL)
    UART_HandleTypeDef* uartHandler = nullptr;
  #endif

  #if CHRONOLOG_THREAD_SAFE
    static void threadSafeLock();
    static void threadSafeUnlock();
  #endif // CHRONOLOG_THREAD_SAFE

  static const char* getCurrentTaskName();
  void getTimeStamp(char* buffer, size_t len) const;
  void printInfo(const char* levelStr, const char* color, const char* time_buf, const char* taskName) const;
  void print(const char* levelStr, const char* color, const char* fmt, va_list args) const;

  };

#else  // CHRONOLOG_MODE

class ChronoLogger {
public:
  constexpr ChronoLogger(const char* moduleName, ChronoLogLevel level = CHRONOLOG_LEVEL_NONE) {}
  void setLevel(ChronoLogLevel level) {}
  void info(const char* fmt, ...) const {}
  void warn(const char* fmt, ...) const {}
  void debug(const char* fmt, ...) const {}
  void error(const char* fmt, ...) const {}
  void fatal(const char* fmt, ...) const {}

  #if CHRONOLOG_PRO_FEATURES
    void progress(uint32_t current, uint32_t total, const char* title) const {}

    void plot(const char* series, float value, uint32_t timeWindowSec = 0) const {}
    void plot(const char* series, const float* values, size_t count, uint32_t timeWindowSec = 0) const {}
    void plotWindow(const char* series) const {}
    void plotWindow() const {}
  #endif // CHRONOLOG_PRO_FEATURES

private:
  #if CHRONOLOG_PRO_FEATURES
    void printProgress(const char* levelStr, const char* color, uint32_t current, uint32_t total, const char* title) const;
  #endif // CHRONOLOG_PRO_FEATURES
};

#endif // CHRONOLOG_MODE

#endif // CHRONOLOG_H