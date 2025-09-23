/*
 ====================================================================================================
 * File:        ChronoLog.h
 * Author:      Hamas Saeed
 * Version:     Rev_1.0.0
 * Date:        Sep 20 2025
 * Brief:       This file provides Debuging functionalities
 * 
 ====================================================================================================
 * License: 
 * MIT License
 * 
 * Copyright (c) 2025 Hamas Saeed
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * For any inquiries, contact Hamas Saeed at hamasaeed@gmail.com
 *
 ====================================================================================================
 */

#ifndef CHRONOLOG_H
#define CHRONOLOG_H

#if defined(ARDUINO)                                                                                       // Platform detection
  #define CHRONOLOG_PLATFORM_ARDUINO
#elif defined(ESP_PLATFORM)
  #define CHRONOLOG_PLATFORM_ESP_IDF
#elif defined(__ZEPHYR__)
  #define CHRONOLOG_PLATFORM_ZEPHYR
#elif defined(STM32F0) || defined(STM32F1) || defined(STM32F3) || defined(STM32F4) || \
      defined(STM32F7) || defined(STM32G0) || defined(STM32G4) || defined(STM32H7) || \
      defined(STM32L0) || defined(STM32L1) || defined(STM32L4) || defined(STM32L5) || \
      defined(STM32WB) || defined(STM32WL)
  #define CHRONOLOG_PLATFORM_STM32_HAL
#endif

#if defined(CHRONOLOG_PLATFORM_ARDUINO)
  #include <Arduino.h>
#if defined(ESP32) || defined(ESP8266)
  #define CHRONOLOG_ESP_FREERTOS
  #include <freertos/task.h>
  #include <freertos/FreeRTOS.h>
#endif
#elif defined(CHRONOLOG_PLATFORM_ESP_IDF)
  #include <time.h>
  #include <esp_log.h>
  #include <sys/time.h>
  #include <freertos/task.h>
  #include <freertos/FreeRTOS.h>
#elif defined(CHRONOLOG_PLATFORM_ZEPHYR)
  extern "C" {
    #include <time.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdarg.h>
    #include <string.h>
    #include <zephyr/kernel.h>
    #include <zephyr/sys/printk.h>
  }
#elif defined(CHRONOLOG_PLATFORM_STM32_HAL)
  #include "main.h"
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdarg.h>
  #include <string.h>
#if defined(osCMSIS) || defined(FREERTOS)
  #define CHRONOLOG_STM32_FREERTOS
  #include "cmsis_os.h"
#endif
#endif


#define CHRONOLOG_MODE          1
#define CHRONOLOG_BUFFER_LEN    256

#define CHRONOLOG_COLOR_INFO    "\033[92m"
#define CHRONOLOG_COLOR_WARN    "\033[93m"
#define CHRONOLOG_COLOR_ERROR   "\033[91m"
#define CHRONOLOG_COLOR_RESET   "\033[0m"
#define CHRONOLOG_COLOR_DEBUG   "\033[94m"
#define CHRONOLOG_COLOR_FATAL   "\033[95m"

enum ChronoLogLevel {
  CHRONOLOG_LEVEL_NONE,
  CHRONOLOG_LEVEL_FATAL,
  CHRONOLOG_LEVEL_ERROR,
  CHRONOLOG_LEVEL_WARN,
  CHRONOLOG_LEVEL_INFO,
  CHRONOLOG_LEVEL_DEBUG
};

#if CHRONOLOG_MODE

class ChronoLogger {
public:
  constexpr ChronoLogger(const char* moduleName, ChronoLogLevel level = CHRONOLOG_LEVEL_DEBUG)
    : name(moduleName), chronoLogLevel(level) {}

  void setLevel(ChronoLogLevel level)               { chronoLogLevel = level; }

#if defined(CHRONOLOG_PLATFORM_STM32_HAL)
  void setUartHandler(UART_HandleTypeDef* handler)  { uartHandler = handler;  }
#endif

  void debug(const char* fmt, ...) const {
    if (chronoLogLevel >= CHRONOLOG_LEVEL_DEBUG) {
      va_list args;
      va_start(args, fmt);
      print("DEBUG", CHRONOLOG_COLOR_DEBUG, fmt, args);
      va_end(args);
    }
  }

  void info(const char* fmt, ...) const {
    if (chronoLogLevel >= CHRONOLOG_LEVEL_INFO) {
      va_list args;
      va_start(args, fmt);
      print("INFO", CHRONOLOG_COLOR_INFO, fmt, args);
      va_end(args);
    }
  }

  void warn(const char* fmt, ...) const {
    if (chronoLogLevel >= CHRONOLOG_LEVEL_WARN) {
      va_list args;
      va_start(args, fmt);
      print("WARNING", CHRONOLOG_COLOR_WARN, fmt, args);
      va_end(args);
    }
  }

  void error(const char* fmt, ...) const {
    if (chronoLogLevel >= CHRONOLOG_LEVEL_ERROR) {
      va_list args;
      va_start(args, fmt);
      print("ERROR", CHRONOLOG_COLOR_ERROR, fmt, args);
      va_end(args);
    }
  }

  void fatal(const char* fmt, ...) const {
    if (chronoLogLevel >= CHRONOLOG_LEVEL_FATAL) {
      va_list args;
      va_start(args, fmt);
      print("FATAL", CHRONOLOG_COLOR_FATAL, fmt, args);
      va_end(args);
    }
  }

private:
  const char* name;
  ChronoLogLevel chronoLogLevel;

#if defined(CHRONOLOG_PLATFORM_STM32_HAL)
  UART_HandleTypeDef* uartHandler = nullptr;
#endif

  static const char* getCurrentTaskName() {
  #if defined(CHRONOLOG_PLATFORM_STM32_HAL) && defined(CHRONOLOG_STM32_FREERTOS)
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
      const char* name = pcTaskGetName(NULL);
      return (name != nullptr) ? name : "MainTask";
    }
    return "MainTask";
  #elif defined(CHRONOLOG_PLATFORM_ZEPHYR)
    return k_thread_name_get(k_current_get());
  #elif defined(CHRONOLOG_PLATFORM_ESP_IDF) || (defined(CHRONOLOG_PLATFORM_ARDUINO) && defined(CHRONOLOG_ESP_FREERTOS))
    return pcTaskGetName(NULL);
  #else
    return "MainTask";
  #endif
  }

  void print(const char* levelStr, const char* color, const char* fmt, va_list args) const {
    char time_buf[16];

    #if defined(CHRONOLOG_PLATFORM_ARDUINO) || defined(CHRONOLOG_PLATFORM_ESP_IDF)
      struct timeval tv;
      gettimeofday(&tv, NULL);
      struct tm timeinfo;
      localtime_r(&tv.tv_sec, &timeinfo);
      snprintf(time_buf, sizeof(time_buf), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    #elif defined(CHRONOLOG_PLATFORM_ZEPHYR)
      uint64_t ms = k_uptime_get();
      snprintf(time_buf, sizeof(time_buf), "%02llu:%02llu:%02llu",
        (ms / 3600000) % 24, (ms / 60000) % 60, (ms / 1000) % 60);
    #elif defined(CHRONOLOG_PLATFORM_STM32_HAL)
      uint32_t ms = HAL_GetTick();
      snprintf(time_buf, sizeof(time_buf), "%02lu:%02lu:%02lu",
        (ms / 3600000) % 24, (ms / 60000) % 60, (ms / 1000) % 60);
    #endif

    const char* taskName = getCurrentTaskName();

    #if defined(CHRONOLOG_PLATFORM_ARDUINO)
      Serial.printf("%s | %-15s | %s%-8s%s | %-16s | ",
        time_buf, name, color, levelStr, CHRONOLOG_COLOR_RESET, taskName);
    #elif defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_PLATFORM_ESP_IDF)
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

    char msg_buf[CHRONOLOG_BUFFER_LEN];
    va_list args_copy;
    va_copy(args_copy, args);
    int len = vsnprintf(msg_buf, sizeof(msg_buf), fmt, args_copy);
    va_end(args_copy);

    if (len < CHRONOLOG_BUFFER_LEN) {
      #if defined(CHRONOLOG_PLATFORM_ARDUINO)
        Serial.print(msg_buf);
      #elif defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_PLATFORM_ESP_IDF)
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
        #elif defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_PLATFORM_ESP_IDF)
          printf("%s", dynamic_buf);
        #elif defined(CHRONOLOG_PLATFORM_STM32_HAL)
          if (!uartHandler) return;
          HAL_UART_Transmit(uartHandler, (uint8_t*)dynamic_buf, len, HAL_MAX_DELAY);
        #endif

        free(dynamic_buf);
      } else {
        #if defined(CHRONOLOG_PLATFORM_ARDUINO)
          Serial.print("[Log too long: memory error]");
        #elif defined(DEBUG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_PLATFORM_ESP_IDF)
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
    #elif defined(CHRONOLOG_PLATFORM_ZEPHYR) || defined(CHRONOLOG_PLATFORM_ESP_IDF)
      printf("\n");
    #elif defined(CHRONOLOG_PLATFORM_STM32_HAL)
      if (!uartHandler) return;
      const char* newline = "\n";
      HAL_UART_Transmit(uartHandler, (uint8_t*)newline, strlen(newline), HAL_MAX_DELAY);
    #endif
  }
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
};

#endif // CHRONOLOG_MODE

#endif // CHRONOLOG_H