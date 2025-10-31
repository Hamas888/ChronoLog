/*
 ============================================================================================================================================
 * File:        ChronoLog.h
 * Author:      Hamas Saeed
 * Version:     Rev_1.1.1
 * Date:        Oct 18 2025
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

#if defined(ARDUINO)                                        // Platform detection
  #include <Arduino.h>
  #if defined(ESP32)
    #define CHRONOLOG_ESP
    #include <freertos/task.h>
    #include <freertos/FreeRTOS.h>
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
  #define CHRONOLOG_MODE          1                         // Set to 0 to disable logging
#endif // CHRONOLOG_MODE

#ifndef CHRONOLOG_BUFFER_LEN
  #define CHRONOLOG_BUFFER_LEN    100                       // Buffer length for formatted messages (increase if needed)
#endif // CHRONOLOG_BUFFER_LEN

#ifndef CHRONOLOG_THREAD_SAFE
  #define CHRONOLOG_THREAD_SAFE   1                         // Set to 0 to disable thread safety (if not using RTOS)
#endif // CHRONOLOG_THREAD_SAFE

#ifndef CHRONOLOG_DEFAULT_LEVEL
#define CHRONOLOG_DEFAULT_LEVEL   CHRONOLOG_LEVEL_DEBUG     // Default log level (can be overridden per instance)
#endif

#ifndef CHRONOLOG_COLOR_ENABLE
  #define CHRONOLOG_COLOR_ENABLE  1                         // Set to 0 to disable colored output (if not supported by terminal)
#endif // CHRONOLOG_COLOR_ENABLE

#ifndef CHRONOLOG_PRO_FEATURES
  #define CHRONOLOG_PRO_FEATURES  0                         // Set to 1 to enable Pro features (e.g. progress start/stop)
#endif // CHRONOLOG_PRO_FEATURES

#ifndef CHRONOLOG_REMOTE_ENABLE
  #define CHRONOLOG_REMOTE_ENABLE 0                         // Set to 1 to enable remote logging
#endif // CHRONOLOG_REMOTE_ENABLE

#if CHRONOLOG_REMOTE_ENABLE
  #include "ChronoLogRemote.h"
#endif // CHRONOLOG_REMOTE_ENABLE

#if CHRONOLOG_COLOR_ENABLE
#define CHRONOLOG_COLOR_INFO    "\033[3m\033[92m"           // Italic + Green
#define CHRONOLOG_COLOR_WARN    "\033[3m\033[93m"           // Italic + Yellow
#define CHRONOLOG_COLOR_ERROR   "\033[3m\033[91m"           // Italic + Red
#define CHRONOLOG_COLOR_DEBUG   "\033[3m\033[94m"           // Italic + Blue
#define CHRONOLOG_COLOR_FATAL   "\033[3m\033[95m"           // Italic + Magenta
#define CHRONOLOG_COLOR_RESET   "\033[0m"                   // Reset

#if CHRONOLOG_PRO_FEATURES
  #define CHRONOLOG_COLOR_PROGF   "\033[3m\033[96m"         // Italic + Cyan
  #define CHRONOLOG_COLOR_PROGS   "\033[3m\033[38;5;208m"   // Italic + Orange
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
    static std::mutex chronoLogMutex;
  #elif defined(CHRONOLOG_PLATFORM_ZEPHYR)
    static struct k_mutex chronoLogMutex;
  #elif defined(CHRONOLOG_PLATFORM_STM32_HAL)
    #if defined(CHRONOLOG_STM32_FREERTOS)
      static SemaphoreHandle_t chronoLogMutex = nullptr;
    #endif
  #elif defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_ESP)
    static SemaphoreHandle_t chronoLogMutex = nullptr;
  #endif
#endif // CHRONOLOG_THREAD_SAFE

#if CHRONOLOG_MODE

class ChronoLogger {
public:
  ChronoLogger(const char* moduleName, ChronoLogLevel level = CHRONOLOG_DEFAULT_LEVEL)
    : name(moduleName), chronoLogLevel(level) {
    #if CHRONOLOG_THREAD_SAFE
      #if defined(CHRONOLOG_PLATFORM_STM32_HAL) && defined(CHRONOLOG_STM32_FREERTOS)
        if (chronoLogMutex == nullptr)  chronoLogMutex = xSemaphoreCreateMutex();
      #elif defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_ESP)
        portMUX_TYPE chronoLogMux = portMUX_INITIALIZER_UNLOCKED;
        if (chronoLogMutex == nullptr) {
          taskENTER_CRITICAL(&chronoLogMux);
          if (chronoLogMutex == nullptr)
            chronoLogMutex = xSemaphoreCreateMutex();
          taskEXIT_CRITICAL(&chronoLogMux);
        }
      #elif defined(CHRONOLOG_PLATFORM_ZEPHYR)
        static bool initialized = false;
        if (!initialized) {
          k_mutex_init(&chronoLogMutex);
          initialized = true;
        }
      #endif
    #endif
  }

  void setLevel(ChronoLogLevel level)               { chronoLogLevel = level; }

#if defined(CHRONOLOG_PLATFORM_STM32_HAL)
  void setUartHandler(UART_HandleTypeDef* handler)  { uartHandler = handler;  }
#endif // CHRONOLOG_PLATFORM_STM32_HAL

  void info(const char* fmt, ...) const { if (chronoLogLevel >= CHRONOLOG_LEVEL_INFO) {
    va_list args;   va_start(args, fmt);  print("INFO", CHRONOLOG_COLOR_INFO, fmt, args);     va_end(args);
  }}

  void warn(const char* fmt, ...) const { if (chronoLogLevel >= CHRONOLOG_LEVEL_WARN) {
    va_list args;   va_start(args, fmt);  print("WARNING", CHRONOLOG_COLOR_WARN, fmt, args);  va_end(args);
  }}

  void debug(const char* fmt, ...) const { if (chronoLogLevel >= CHRONOLOG_LEVEL_DEBUG) { 
    va_list args;   va_start(args, fmt);  print("DEBUG", CHRONOLOG_COLOR_DEBUG, fmt, args);   va_end(args);
  }}

  void error(const char* fmt, ...) const { if (chronoLogLevel >= CHRONOLOG_LEVEL_ERROR) {
    va_list args;   va_start(args, fmt);  print("ERROR", CHRONOLOG_COLOR_ERROR, fmt, args);   va_end(args);
  }}

  void fatal(const char* fmt, ...) const { if (chronoLogLevel >= CHRONOLOG_LEVEL_FATAL) {
    va_list args;   va_start(args, fmt);  print("FATAL", CHRONOLOG_COLOR_FATAL, fmt, args);   va_end(args);
  }}

  #if CHRONOLOG_PRO_FEATURES
    void progress(const uint32_t current, const uint32_t total, const char* title) const {
      if (chronoLogLevel >= CHRONOLOG_LEVEL_PRO_FEATURES) {
        uint32_t  tempCurrent = (current > total) ? total : current;
        uint32_t  tempTotal   = (total == 0) ? 1 : total; // Prevent division by zero
        if(current < total) { 
          printProgress("PROGRESS", CHRONOLOG_COLOR_PROGS, tempCurrent, tempTotal, title);
        } else { 
          printProgress("PROGRESS", CHRONOLOG_COLOR_PROGF, tempCurrent, tempTotal, title);
          #if defined(CHRONOLOG_PLATFORM_ARDUINO)
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
      }
    }
  #endif // CHRONOLOG_PRO_FEATURES

private:
  const char*           name;
  ChronoLogLevel        chronoLogLevel;

  #if defined(CHRONOLOG_PLATFORM_STM32_HAL)
    UART_HandleTypeDef* uartHandler = nullptr;
  #endif

  #if CHRONOLOG_THREAD_SAFE
    static void threadSafeLock() {
      #if defined(CHRONOLOG_PLATFORM_STM32_HAL) && defined(CHRONOLOG_STM32_FREERTOS)
        if (chronoLogMutex) xSemaphoreTake(chronoLogMutex, portMAX_DELAY);
      #elif defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_ESP)
        if (chronoLogMutex) xSemaphoreTake(chronoLogMutex, portMAX_DELAY);
      #elif defined(CHRONOLOG_PLATFORM_ZEPHYR)
        k_mutex_lock(&chronoLogMutex, K_FOREVER);
      #elif defined(CHRONOLOG_PLATFORM_DESKTOP)
        chronoLogMutex.lock();
      #endif
    }

    static void threadSafeUnlock() {
      #if defined(CHRONOLOG_PLATFORM_STM32_HAL) && defined(CHRONOLOG_STM32_FREERTOS)
        if (chronoLogMutex) xSemaphoreGive(chronoLogMutex);
      #elif defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_ESP)
        if (chronoLogMutex) xSemaphoreGive(chronoLogMutex);
      #elif defined(CHRONOLOG_PLATFORM_ZEPHYR)
        k_mutex_unlock(&chronoLogMutex);
      #elif defined(CHRONOLOG_PLATFORM_DESKTOP)
        chronoLogMutex.unlock();
      #endif
    }
  #endif // CHRONOLOG_THREAD_SAFE

  static const char* getCurrentTaskName() {
  #if defined(CHRONOLOG_PLATFORM_STM32_HAL) && defined(CHRONOLOG_STM32_FREERTOS)
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
      const char* name = pcTaskGetName(NULL);
      return (name != nullptr) ? name : "MainTask";
    }
    return "MainTask";
  #elif defined(CHRONOLOG_PLATFORM_DESKTOP)
    static thread_local char threadName[32];
    snprintf(threadName, sizeof(threadName), "Thread-%zu",
            std::hash<std::thread::id>{}(std::this_thread::get_id()) % 1000);
    return threadName;
  #elif defined(CHRONOLOG_PLATFORM_ZEPHYR)
    return k_thread_name_get(k_current_get());
  #elif defined(CHRONOLOG_PLATFORM_ESP_IDF) || (defined(CHRONOLOG_PLATFORM_ARDUINO) && defined(CHRONOLOG_ESP))
    return pcTaskGetName(NULL);
  #else
    return "MainTask";
  #endif
  }

  void getTimeStamp(char* buffer, size_t len) const {
    #if defined(CHRONOLOG_PLATFORM_ESP_IDF) || (defined(CHRONOLOG_PLATFORM_ARDUINO) && defined(CHRONOLOG_ESP))
      struct timeval tv;
      gettimeofday(&tv, NULL);
      struct tm timeinfo;
      localtime_r(&tv.tv_sec, &timeinfo);
      snprintf(buffer, len, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
      return;
    #elif defined(CHRONOLOG_PLATFORM_DESKTOP)
      time_t now = time(NULL);
      struct tm *t = localtime(&now);
      snprintf(buffer, len, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
    #elif defined(CHRONOLOG_PLATFORM_ZEPHYR)
      uint64_t ms = k_uptime_get();
      snprintf(buffer, len, "%02llu:%02llu:%02llu",
        (ms / 3600000) % 24, (ms / 60000) % 60, (ms / 1000) % 60);
    #elif defined(CHRONOLOG_PLATFORM_STM32_HAL)
      uint32_t ms = HAL_GetTick();
      snprintf(buffer, len, "%02lu:%02lu:%02lu",
        (ms / 3600000) % 24, (ms / 60000) % 60, (ms / 1000) % 60);
    #elif (defined(CHRONOLOG_PLATFORM_ARDUINO) && !defined(CHRONOLOG_ESP))
      unsigned long ms = millis();
      snprintf(buffer, len, "%02lu:%02lu:%02lu",
        (ms / 3600000) % 24, (ms / 60000) % 60, (ms / 1000) % 60);
    #endif
  }

  void printInfo(const char* levelStr, const char* color) const {
    char time_buf[16];
    getTimeStamp(time_buf, 16);

    const char* taskName = getCurrentTaskName();

    #if defined(CHRONOLOG_PLATFORM_ARDUINO) && defined(CHRONOLOG_ESP)
      Serial.printf("%s | %-15s | %s%-8s%s | %-16s | ",
        time_buf, name, color, levelStr, CHRONOLOG_COLOR_RESET, taskName);
    #elif defined(CHRONOLOG_PLATFORM_ARDUINO) && !defined(CHRONOLOG_ESP)
      char line_buf[96];
      snprintf(line_buf, sizeof(line_buf),
               "%s | %-15s | %s%-8s%s | %-16s | ",
               time_buf, name, color, levelStr, CHRONOLOG_COLOR_RESET, taskName);
      Serial.print(line_buf);
    #elif defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_PLATFORM_DESKTOP)
      printf("%s | %-15s | %s%-8s%s | %-16s | ",
        time_buf, name, color, levelStr, CHRONOLOG_COLOR_RESET, taskName);
    #elif defined(CHRONOLOG_PLATFORM_STM32_HAL)
      char line_buf[96];
      snprintf(line_buf, sizeof(line_buf),
               "%s | %-15s | %s%-8s%s | %-16s | ",
               time_buf, name, color, levelStr, CHRONOLOG_COLOR_RESET, taskName);
      if (!uartHandler) return;
      HAL_UART_Transmit(uartHandler, (uint8_t*)line_buf, strlen(line_buf), HAL_MAX_DELAY);
    #endif
  }

  void print(const char* levelStr, const char* color, const char* fmt, va_list args) const {
    #if CHRONOLOG_THREAD_SAFE
      threadSafeLock();
    #endif

    printInfo(levelStr, color);
    char msg_buf[CHRONOLOG_BUFFER_LEN];
    va_list args_copy;
    va_copy(args_copy, args);
    int len = vsnprintf(msg_buf, sizeof(msg_buf), fmt, args_copy);
    va_end(args_copy);

    if (len < CHRONOLOG_BUFFER_LEN) {
      #if defined(CHRONOLOG_PLATFORM_ARDUINO)
        Serial.print(msg_buf);
      #elif defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_PLATFORM_ESP_IDF)  || defined(CHRONOLOG_PLATFORM_DESKTOP)
        printf("%s", msg_buf);
      #elif defined(CHRONOLOG_PLATFORM_STM32_HAL)
        if (!uartHandler) return;
        HAL_UART_Transmit(uartHandler, (uint8_t*)msg_buf, len, HAL_MAX_DELAY);
      #endif
    } else {
      char* dynamic_buf = (char*)malloc(len + 1);
      if (dynamic_buf) {
        va_copy(args_copy, args);
        vsnprintf(dynamic_buf, len + 1, fmt, args_copy);
        va_end(args_copy);

        #if defined(CHRONOLOG_PLATFORM_ARDUINO)
          Serial.print(dynamic_buf);
        #elif defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_PLATFORM_DESKTOP)  
          printf("%s", dynamic_buf);
        #elif defined(CHRONOLOG_PLATFORM_STM32_HAL)
          if (!uartHandler) return;
          HAL_UART_Transmit(uartHandler, (uint8_t*)dynamic_buf, len, HAL_MAX_DELAY);
        #endif

        free(dynamic_buf);
      } else {
        #if defined(CHRONOLOG_PLATFORM_ARDUINO)
          Serial.print("[Log too long: memory error]");
        #elif defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_PLATFORM_DESKTOP)
          printf("[Log too long: memory error]");
        #elif defined(CHRONOLOG_PLATFORM_STM32_HAL)
          if (!uartHandler) return;
          const char* err_msg = "[Log too long: memory error]";
          HAL_UART_Transmit(uartHandler, (uint8_t*)err_msg, strlen(err_msg), HAL_MAX_DELAY);
        #endif
      }
    }

    #if defined(CHRONOLOG_PLATFORM_ARDUINO)
      Serial.println();
    #elif defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_PLATFORM_DESKTOP)
      printf("\n");
    #elif defined(CHRONOLOG_PLATFORM_STM32_HAL)
      if (!uartHandler) return;
      const char* newline = "\n";
      HAL_UART_Transmit(uartHandler, (uint8_t*)newline, strlen(newline), HAL_MAX_DELAY);
    #endif

    #if CHRONOLOG_THREAD_SAFE
      threadSafeUnlock();
    #endif

    // Send to remote logging if enabled
    #if CHRONOLOG_REMOTE_ENABLE
    char full_buf[CHRONOLOG_BUFFER_LEN * 2];
    char time_buf[16];
    getTimeStamp(time_buf, 16);
    const char* taskName = getCurrentTaskName();
    
    snprintf(full_buf, sizeof(full_buf), "%s | %-15s | %s%-8s%s | %-16s | %s\n",
             time_buf, name, color, levelStr, CHRONOLOG_COLOR_RESET, taskName, msg_buf);
             
    ChronoLogRemote::getInstance()->write(full_buf);
    #endif
  }

  #if CHRONOLOG_PRO_FEATURES
    void printProgress(const char* levelStr, const char* color, uint32_t current, uint32_t total, const char* title) const {
      #if CHRONOLOG_THREAD_SAFE
        threadSafeLock();
      #endif
      printInfo(levelStr, color);
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
      #if defined(CHRONOLOG_PLATFORM_ARDUINO)
        Serial.print(prog_buf);
        Serial.print("\r");  // Carriage return to overwrite same line
        Serial.flush();      // Ensure output is sent immediately
      #elif defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_PLATFORM_ESP_IDF) || defined(CHRONOLOG_PLATFORM_DESKTOP)
        printf("%s\r", prog_buf);  // Carriage return to overwrite same line
        fflush(stdout);            // Ensure output is sent immediately
      #elif defined(CHRONOLOG_PLATFORM_STM32_HAL)
        if (!uartHandler) return;
        HAL_UART_Transmit(uartHandler, (uint8_t*)prog_buf, strlen(prog_buf), HAL_MAX_DELAY);
        const char* carriage_return = "\r";
        HAL_UART_Transmit(uartHandler, (uint8_t*)carriage_return, strlen(carriage_return), HAL_MAX_DELAY);
      #endif
      #if CHRONOLOG_THREAD_SAFE
        threadSafeUnlock();
      #endif
    }
  #endif
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
  #endif // CHRONOLOG_PRO_FEATURES
};

#endif // CHRONOLOG_MODE

#endif // CHRONOLOG_H