# ChronoLog 🕒

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Version](https://img.shields.io/badge/Version-1.0.0-green.svg)](https://github.com/Hamas888/ChronoLog)
[![Platform](https://img.shields.io/badge/Platform-ARDUINO%20|%20ESP32%20|%20STM32%20|%20nRF52%20|%20Linux%20|%20Windows%20|%20MacOS%20-orange.svg)](https://github.com/Hamas888/ChronoLog)

A **cross-platform real-time logging library** for embedded systems that provides structured, colorized, and timestamped logging with automatic platform detection. ChronoLog seamlessly adapts to different embedded environments including Arduino, ESP-IDF, nRF Connect SDK (Zephyr), and STM32 HAL with or without RTOS support.

## 📑 Table of Contents

- [✨ Features](#-features)
- [🎯 Supported Platforms](#-supported-platforms)
- [📦 Installation](#-installation)
  - [PlatformIO (Arduino & ESP-IDF)](#platformio-arduino--esp-idf)
  - [ESP-IDF Component](#esp-idf-component)
  - [nRF Connect SDK (Zephyr)](#nrf-connect-sdk-zephyr)
  - [STM32Cube Project](#stm32cube-project)
- [🚀 Quick Start](#-quick-start)
  - [Basic Usage](#basic-usage)
  - [Multiple Module Loggers](#multiple-module-loggers)
  - [Runtime Log Level Control](#runtime-log-level-control)
- [📋 Log Output Examples](#-log-output-examples)
- [⚙️ Configuration](#️-configuration)
- [🛠️ Platform-Specific Requirements](#️-platform-specific-requirements)
- [📁 Repository Structure](#-repository-structure)
- [🤝 Contributing](#-contributing)
- [📄 License](#-license)
- [👨‍💻 Author](#-author)
- [⭐ Show Your Support](#-show-your-support)

## ✨ Features

- **🎯 Automatic Platform Detection**: Detects and adapts to Arduino, ESP-IDF, Zephyr (nRF Connect SDK), and STM32 HAL environments
- **⏰ Smart Timestamps**: 
  - Real-time timestamps when system time is synced (Arduino/ESP-IDF with NTP)
  - Uptime-based timestamps for other platforms
- **🎨 Colorized Output**: Color-coded log levels for better readability
- **📋 Structured Logging**: Clean tabular format with timestamps, module names, log levels, and thread/task information
- **🧵 RTOS/Thread Aware**: Automatically displays current task/thread names
- **📊 Multiple Log Levels**: DEBUG, INFO, WARN, ERROR, FATAL with runtime level control
- **🎛️ Module-based Logging**: Create separate loggers for different modules with individual log levels
- **💾 Memory Efficient**: Header-only library with minimal memory footprint
- **🚀 Zero Configuration**: Works out-of-the-box on supported platforms

## 🎯 Supported Platforms

| Platform | Framework | RTOS Support | Real-time Clock |
|----------|-----------|--------------|-----------------|
| **ESP32** | Arduino | ✅ FreeRTOS | ✅ NTP Sync |
| **ESP32** | ESP-IDF | ✅ FreeRTOS | ✅ NTP Sync |
| **STM32** | HAL | ✅ FreeRTOS/CMSIS-OS | ⏱️ HAL_GetTick() |
| **STM32** | HAL | ❌ Bare Metal | ⏱️ HAL_GetTick() |
| **nRF52** | nRF Connect SDK | ✅ Zephyr | ⏱️ k_uptime_get() |

## 📦 Installation

### PlatformIO (Arduino & ESP-IDF)

Simply add the GitHub repository to your `platformio.ini` file:

```ini
[env:your_board]
platform = your_platform
board = your_board
framework = arduino  ; or espidf

lib_deps = https://github.com/Hamas888/ChronoLog.git
```

**Example for ESP32-C6:**
```ini
[env:esp32-c6-devkitm-1]
platform = https://github.com/tasmota/platform-espressif32.git
board = esp32-c6-devkitm-1
framework = arduino

lib_deps = https://github.com/Hamas888/ChronoLog.git
```

### ESP-IDF Component

1. Create a `components` folder in your project root (if it doesn't exist)
2. Navigate to the components folder and clone the repository:
   ```bash
   cd components
   git clone https://github.com/Hamas888/ChronoLog.git
   ```
3. Add the requirement to your main `CMakeLists.txt`:
   ```cmake
   idf_component_register(
       SRCS "main.cpp"
       INCLUDE_DIRS "."
       REQUIRES ChronoLog  # Add this line
   )
   ```

### nRF Connect SDK (Zephyr)

1. Create a `modules` folder in your project root
2. Navigate to the modules folder and clone the repository:
   ```bash
   cd modules
   git clone https://github.com/Hamas888/ChronoLog.git
   ```
3. Add the subdirectory to your root `CMakeLists.txt`:
   ```cmake
   add_subdirectory(modules/ChronoLog)
   target_link_libraries(app PRIVATE ChronoLog)
   ```

### STM32Cube Project

1. Clone the repository in your project root:
   ```bash
   git clone https://github.com/Hamas888/ChronoLog.git
   ```
2. Add to your root `CMakeLists.txt` after `add_executable()`:
   ```cmake
   add_executable(${PROJECT_NAME} ${SOURCES} ${LINKER_SCRIPT})
   
   # Add ChronoLog
   add_subdirectory(ChronoLog)
   target_link_libraries(${PROJECT_NAME} ChronoLog)
   ```

## 🚀 Quick Start

### Basic Usage

```cpp
#include "ChronoLog.h"

// Create a logger for your module
ChronoLogger logger("MyModule", CHRONOLOG_LEVEL_DEBUG);

void setup() {  // or main() for non-Arduino platforms
    // For STM32, set UART handler (only required for STM32)
    #ifdef STM32
    logger.setUartHandler(&huart2);  // Your UART handle
    #endif
    
    logger.info("System initialized");
    logger.debug("Debug information");
    logger.warn("This is a warning");
    logger.error("An error occurred");
    logger.fatal("Critical system error");
}
```

### Multiple Module Loggers

```cpp
#include "ChronoLog.h"

ChronoLogger sensorLogger("Sensors", CHRONOLOG_LEVEL_INFO);
ChronoLogger networkLogger("Network", CHRONOLOG_LEVEL_DEBUG);  
ChronoLogger systemLogger("System", CHRONOLOG_LEVEL_WARN);

void sensorTask() {
    sensorLogger.info("Reading temperature: %.2f°C", 25.6);
    sensorLogger.debug("Sensor calibration complete");
}

void networkTask() {
    networkLogger.info("WiFi connected to %s", ssid);
    networkLogger.error("Failed to connect to server");
}
```

### Runtime Log Level Control

```cpp
ChronoLogger logger("MyModule", CHRONOLOG_LEVEL_INFO);

void updateLogLevel(int level) {
    logger.setLevel(static_cast<ChronoLogLevel>(level));
    logger.info("Log level updated to %d", level);
}
```

## 📋 Log Output Examples

### Arduino/ESP-IDF with NTP Sync
```
14:32:15 | Sensors         | INFO     | WiFiTask         | Temperature reading: 25.6°C
14:32:15 | Network         | DEBUG    | NetworkTask      | Attempting connection to server
14:32:16 | Network         | WARN     | NetworkTask      | Connection timeout, retrying...
14:32:17 | System          | ERROR    | MainTask         | Memory allocation failed
14:32:17 | System          | FATAL    | ErrorHandler     | System halt required
```

### STM32/nRF52 with Uptime
```
00:05:23 | Sensors         | INFO     | SensorTask       | Accelerometer initialized
00:05:23 | Sensors         | DEBUG    | SensorTask       | Calibration data: X=1.02, Y=0.98, Z=9.81
00:05:24 | BLE             | INFO     | BLETask          | Advertisement started
00:05:25 | System          | WARN     | MainTask         | Low battery warning: 15%
```

### Color-Coded Output
The library automatically applies color coding to different log levels:
- 🔵 **DEBUG**: Blue
- 🟢 **INFO**: Green  
- 🟡 **WARN**: Yellow
- 🔴 **ERROR**: Red
- 🟣 **FATAL**: Magenta

## ⚙️ Configuration

### Compile-Time Configuration

```cpp
// Disable logging completely (for production)
#define CHRONOLOG_MODE 0  // Set to 0 to disable all logging

// Adjust buffer size for long messages
#define CHRONOLOG_BUFFER_LEN 512  // Default is 256
```

### Log Levels

```cpp
enum ChronoLogLevel {
    CHRONOLOG_LEVEL_NONE,   // No logging
    CHRONOLOG_LEVEL_FATAL,  // Only fatal errors
    CHRONOLOG_LEVEL_ERROR,  // Errors and above
    CHRONOLOG_LEVEL_WARN,   // Warnings and above  
    CHRONOLOG_LEVEL_INFO,   // Info and above (recommended)
    CHRONOLOG_LEVEL_DEBUG   // All messages (development)
};
```

## 🛠️ Platform-Specific Requirements

### Arduino (ESP32)
- No additional setup required
- Serial output at configured baud rate
- For real timestamps, sync system time with NTP

### ESP-IDF
- No additional setup required  
- Output to stdout/console
- For real timestamps, configure SNTP

### STM32 HAL
- **Required**: Set UART handler using `logger.setUartHandler(&huartX)`
- Works with or without FreeRTOS/CMSIS-OS
- Output via specified UART peripheral

### nRF Connect SDK (Zephyr)
- No additional setup required
- Uses Zephyr's printk for output
- Automatic thread name detection

## 📁 Repository Structure

```
ChronoLog/
├── include/
│   └── ChronoLog.h          # Main header file
├── examples/
│   ├── PlatformIO/
│   │   ├── Arduino/         # Arduino framework examples
│   │   └── ESP-IDF/         # ESP-IDF framework examples  
│   ├── ESP-IDF/             # Native ESP-IDF examples
│   ├── STM32Cube/           # STM32CubeIDE examples
│   └── NRF Connect SDK/     # nRF Connect SDK examples
├── CMakeLists.txt           # Multi-platform CMake configuration
├── library.json             # PlatformIO library manifest
├── library.properties       # Arduino library properties
├── idf_component.yml        # ESP-IDF component configuration
└── README.md
```

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

This permissive license allows for both commercial and non-commercial use, making ChronoLog suitable for any embedded project.

## 👨‍💻 Author

**Hamas Saeed**
- GitHub: [@Hamas888](https://github.com/Hamas888)  
- Email: hamasaeed@gmail.com

## ⭐ Show Your Support

If this project helped you, please give it a ⭐️ on [GitHub](https://github.com/Hamas888/ChronoLog)!
