# ChronoLog 🕒

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Version](https://img.shields.io/badge/Version-1.1.0-green.svg)](https://github.com/Hamas888/ChronoLog)
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
- [🔧 Troubleshooting](#-troubleshooting)
- [📁 Repository Structure](#-repository-structure)
- [🛣️ Roadmap & Upcoming Features](#️-roadmap--upcoming-features)
- [💖 Support & Motivation](#-support--motivation)
- [🤝 Contributing](#-contributing)
- [📄 License](#-license)
- [👨‍💻 Author](#-author)
- [⭐ Show Your Support](#-show-your-support)

## ✨ Features

- **🎯 Automatic Platform Detection**: Detects and adapts to Arduino, ESP-IDF, Zephyr (nRF Connect SDK), STM32 HAL, and desktop environments
- **⏰ Smart Timestamps**: 
  - Real-time timestamps when system time is synced (Arduino/ESP-IDF with NTP)
  - Uptime-based timestamps for other platforms
  - System time for desktop platforms (Linux, Windows, macOS)
- **🎨 Colorized Output**: Color-coded log levels for better readability
- **📋 Structured Logging**: Clean tabular format with timestamps, module names, log levels, and thread/task information
- **🧵 Thread-Safe Operation**: Safe concurrent logging from multiple threads
- **📊 Multiple Log Levels**: DEBUG, INFO, WARN, ERROR, FATAL with runtime level control
- **🎛️ Module-based Logging**: Create separate loggers for different modules with individual log levels
- **📈 Progress Bar Support**: Built-in progress tracking with visual progress bars (requires `CHRONOLOG_PRO_FEATURES`)
- **🖥️ Desktop Support**: Full compatibility with Linux, Windows, and macOS
- **💾 Memory Efficient**: Header-only library with minimal memory footprint
- **🚀 Zero Configuration**: Works out-of-the-box on supported platforms
- **⚙️ Conditional Compilation**: Enable only the features you need to optimize for your platform

## 🎯 Supported Platforms

| Platform | Framework | RTOS/Thread Support | Real-time Clock |
|----------|-----------|---------------------|-----------------|
| **ESP32** | Arduino | ✅ FreeRTOS | ✅ NTP Sync |
| **ESP32** | ESP-IDF | ✅ FreeRTOS | ✅ NTP Sync |
| **STM32** | HAL | ✅ FreeRTOS/CMSIS-OS | ⏱️ HAL_GetTick() |
| **STM32** | HAL | ❌ Bare Metal | ⏱️ HAL_GetTick() |
| **nRF52** | nRF Connect SDK | ✅ Zephyr | ⏱️ k_uptime_get() |
| **Linux** | Native | ✅ pthread | ✅ System Time |
| **Windows** | Native | ✅ Windows Threads | ✅ System Time |
| **macOS** | Native | ✅ pthread | ✅ System Time |

## 📦 Installation

### PlatformIO (Arduino & ESP-IDF)

#### Method 1: Library Manager Search
Search for "ChronoLog" in the PlatformIO Library Manager and add it to your project.

#### Method 2: Add to platformio.ini
Simply add ChronoLog to the `lib_deps` section in your `platformio.ini` file:

```ini
[env:your_board]
platform = your_platform
board = your_board
framework = arduino  ; or espidf

lib_deps = 
    ChronoLog
```

**Example for ESP32-C6:**
```ini
[env:esp32-c6-devkitm-1]
platform = https://github.com/tasmota/platform-espressif32.git
board = esp32-c6-devkitm-1
framework = arduino

lib_deps = 
    ChronoLog
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

### Desktop Applications (Linux, Windows, macOS)

For native desktop applications, simply include the library in your CMake project:

1. Clone or download ChronoLog to your project:
   ```bash
   git clone https://github.com/Hamas888/ChronoLog.git
   ```

2. Add to your `CMakeLists.txt`:
   ```cmake
   add_subdirectory(ChronoLog)
   target_link_libraries(your_target_name ChronoLog)
   ```

**Note**: Desktop builds automatically detect the platform and provide full thread safety and system time support.

## 🚀 Quick Start

### Basic Usage

```cpp
#include "ChronoLog.h"

// Create a logger for your module
ChronoLogger logger("MyModule", CHRONOLOG_LEVEL_DEBUG);

void setup() {  // or main() for non-Arduino platforms
    // For STM32, set UART handler (REQUIRED for STM32 platforms)
    #if defined(CHRONOLOG_PLATFORM_STM32_HAL)
        logger.setUartHandler(&huart2);  // Your UART handle (e.g., huart1, huart2)
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

void initLoggers() {
    // Required for STM32 platforms
    #if defined(CHRONOLOG_PLATFORM_STM32_HAL)
        sensorLogger.setUartHandler(&huart2);
        networkLogger.setUartHandler(&huart2);
        systemLogger.setUartHandler(&huart2);
    #endif
}

void sensorTask() {
    sensorLogger.info("Reading temperature: %.2f°C", 25.6f);
    sensorLogger.debug("Sensor calibration complete");
}

void networkTask() {
    networkLogger.info("WiFi connected to %s", "MyNetwork");
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

### Desktop Application Example

```cpp
#include "ChronoLog.h"

#if defined(CHRONOLOG_PLATFORM_DESKTOP)
    #include <thread>
    #include <chrono>
#endif

ChronoLogger appLogger("DesktopApp", CHRONOLOG_LEVEL_DEBUG);

void workerThread(int threadId) {
    for (int i = 0; i < 5; i++) {
        appLogger.info("Worker %d processing item %d", threadId, i);
        
        #if defined(CHRONOLOG_PLATFORM_DESKTOP)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        #endif
    }
}

int main() {
    appLogger.info("Desktop application started");
    
    #if defined(CHRONOLOG_PLATFORM_DESKTOP)
        // Create multiple threads for testing thread safety
        std::thread t1(workerThread, 1);
        std::thread t2(workerThread, 2);
        
        t1.join();
        t2.join();
    #else
        // For embedded platforms, run sequentially
        workerThread(1);
        workerThread(2);
    #endif
    
    appLogger.info("All workers completed");
    return 0;
}
```

### Progress Bar Example

```cpp
#include "ChronoLog.h"

// Enable pro features in your build configuration
// #define CHRONOLOG_PRO_FEATURES 1

ChronoLogger logger("Progress", CHRONOLOG_LEVEL_PRO_FEATURES);

void processWithProgress() {
    uint32_t total = 100;
    
    for (uint32_t i = 0; i <= total; i++) {
        // Update progress bar (parameter order: current, total, title)
        logger.progress(i, total, "Processing data");
        
        // Simulate work
        #if defined(CHRONOLOG_PLATFORM_DESKTOP)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        #elif defined(CHRONOLOG_PLATFORM_ARDUINO)
            delay(50);
        #elif defined(CHRONOLOG_PLATFORM_ESP_IDF)
            vTaskDelay(pdMS_TO_TICKS(50));
        #endif
    }
    
    logger.info("Processing completed!");
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

### Desktop Applications with Thread Safety
```
14:32:15 | DesktopApp      | INFO     | Thread-123       | Worker 1 processing item 0
14:32:15 | DesktopApp      | INFO     | Thread-456       | Worker 2 processing item 0
14:32:15 | FileHandler     | DEBUG    | Thread-789       | Opening configuration file
14:32:16 | Database        | ERROR    | Thread-234       | Connection timeout after 5s
14:32:16 | Progress        | PROGRESS | Thread-567       | Processing data: 100% (100/100) [====================]
```

**Note**: Progress bars require `CHRONOLOG_PRO_FEATURES` to be enabled and logger level set to `CHRONOLOG_LEVEL_PRO_FEATURES` or higher.

### Progress Bar Color Coding
Progress bars use conditional coloring based on completion status:
- **🟠 Orange**: In-progress (0-99% complete) 
- **🔵 Cyan**: Completed (100% complete)

### Color-Coded Output
The library automatically applies color coding to different log levels:
- 🔵 **DEBUG**: Blue (Italic)
- 🟢 **INFO**: Green (Italic)
- 🟡 **WARN**: Yellow (Italic)
- 🔴 **ERROR**: Red (Italic)
- 🟣 **FATAL**: Magenta (Italic)
- 🟠 **PROGRESS** (In-progress): Orange (Italic)
- 🔵 **PROGRESS** (Complete): Cyan (Italic)

## ⚙️ Configuration

### Compile-Time Feature Configuration

ChronoLog allows you to enable or disable features at compile time to optimize for your specific use case:

```cpp
// === Core Logging Configuration ===
#define CHRONOLOG_MODE 1                    // Set to 0 to disable all logging
#define CHRONOLOG_BUFFER_LEN 100            // Buffer size for log messages (default: 100)
#define CHRONOLOG_DEFAULT_LEVEL CHRONOLOG_LEVEL_DEBUG  // Default log level

// === Feature Control ===
#define CHRONOLOG_COLOR_ENABLE 1            // Enable colored output (1 = enabled, 0 = disabled)
#define CHRONOLOG_THREAD_SAFE 1             // Enable thread-safe operations (1 = enabled, 0 = disabled)
#define CHRONOLOG_PRO_FEATURES 1            // Enable Pro features like progress bars (1 = enabled, 0 = disabled)

// Note: The following features are automatically detected based on platform:
// - Timestamps are always enabled and use platform-appropriate time sources
// - Thread/task information is automatically detected on RTOS platforms
// - Desktop support is automatic when compiling for Linux/Windows/macOS
// - RTOS support is automatic when FreeRTOS/Zephyr/CMSIS-OS is detected
```

### Example Configuration for Different Scenarios

#### Minimal Embedded Configuration (Memory Constrained)
```cpp
// Minimal configuration for resource-limited embedded systems
#define CHRONOLOG_MODE 1
#define CHRONOLOG_BUFFER_LEN 64             // Smaller buffer to save memory
#define CHRONOLOG_COLOR_ENABLE 0            // Disable colors to save memory
#define CHRONOLOG_THREAD_SAFE 0             // Disable if not using RTOS
#define CHRONOLOG_PRO_FEATURES 0            // Disable progress bars
#define CHRONOLOG_DEFAULT_LEVEL CHRONOLOG_LEVEL_ERROR  // Only show errors
```

#### Full-Featured Desktop Configuration
```cpp
// Full-featured configuration for desktop applications
#define CHRONOLOG_MODE 1
#define CHRONOLOG_BUFFER_LEN 512            // Larger buffer for detailed messages
#define CHRONOLOG_COLOR_ENABLE 1            // Enable colors for better readability
#define CHRONOLOG_THREAD_SAFE 1             // Essential for multi-threaded desktop apps
#define CHRONOLOG_PRO_FEATURES 1            // Enable progress bars and advanced features
#define CHRONOLOG_DEFAULT_LEVEL CHRONOLOG_LEVEL_DEBUG  // Show all messages during development
```

#### Production Embedded Configuration
```cpp
// Production configuration with essential features only
#define CHRONOLOG_MODE 1
#define CHRONOLOG_BUFFER_LEN 200            // Moderate buffer size
#define CHRONOLOG_COLOR_ENABLE 0            // Disable colors for production
#define CHRONOLOG_THREAD_SAFE 1             // Keep thread safety for RTOS
#define CHRONOLOG_PRO_FEATURES 0            // Disable progress bars in production
#define CHRONOLOG_DEFAULT_LEVEL CHRONOLOG_LEVEL_INFO   // Info level for production
```

### Log Levels

```cpp
enum ChronoLogLevel {
    CHRONOLOG_LEVEL_NONE,   // No logging
    CHRONOLOG_LEVEL_FATAL,  // Only fatal errors
    CHRONOLOG_LEVEL_ERROR,  // Errors and above
    CHRONOLOG_LEVEL_WARN,   // Warnings and above  
    CHRONOLOG_LEVEL_INFO,   // Info and above (recommended)
    CHRONOLOG_LEVEL_DEBUG,  // All messages (development)
    CHRONOLOG_LEVEL_PRO_FEATURES  // Includes progress bars (only when CHRONOLOG_PRO_FEATURES is enabled)
};
```

**Note**: `CHRONOLOG_LEVEL_PRO_FEATURES` is only available when `CHRONOLOG_PRO_FEATURES` is defined as 1. This conditional level enables advanced features like progress bars while maintaining backward compatibility.

### Runtime Configuration

```cpp
ChronoLogger logger("MyModule", CHRONOLOG_LEVEL_INFO);

// Change log level at runtime
logger.setLevel(CHRONOLOG_LEVEL_DEBUG);

// For STM32 platforms, set UART handler
#if defined(CHRONOLOG_PLATFORM_STM32_HAL)
    logger.setUartHandler(&huart2);
#endif

// Note: Color and timestamp settings are compile-time only
// Use the #define directives shown above to control these features
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

**STM32 Complete Setup Example:**
```cpp
#include "ChronoLog.h"

ChronoLogger logger("STM32App", CHRONOLOG_LEVEL_DEBUG);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();  // Initialize your UART
    
    // REQUIRED: Set UART handler for ChronoLog
    logger.setUartHandler(&huart2);
    
    logger.info("STM32 application started");
    
    while (1) {
        logger.debug("Main loop iteration");
        HAL_Delay(1000);
    }
}
```

### nRF Connect SDK (Zephyr)
- No additional setup required
- Uses Zephyr's printk for output
- Automatic thread name detection

### Desktop Platforms (Linux, Windows, macOS)
- **Thread Safety**: Automatic mutex-based synchronization for multi-threaded applications
- **Progress Bars**: Full support for visual progress tracking
- **System Time**: Uses system clock for real-time timestamps
- **File Logging**: Optional file output support (coming in future releases)
- **Colors**: Full ANSI color support in compatible terminals

## 🔧 Troubleshooting

### Common Issues & Solutions

**STM32: No log output visible**
- Ensure `logger.setUartHandler(&huartX)` is called before logging
- Verify UART is properly initialized and connected
- Check baud rate settings match your serial monitor

**Progress bars not working**
- Verify `#define CHRONOLOG_PRO_FEATURES 1` is set
- Ensure logger level is `CHRONOLOG_LEVEL_PRO_FEATURES` or higher
- Progress bars are only available when Pro features are enabled

**Compilation errors on platform detection**
- Platform detection is automatic; avoid manually defining platform macros
- For custom platforms, check the header file for supported detection patterns

**Thread safety issues**
- Thread safety is enabled by default (`CHRONOLOG_THREAD_SAFE 1`)
- On custom RTOS implementations, mutex initialization may need adjustment

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
│   ├── NRF Connect SDK/     # nRF Connect SDK examples
│   ├── Pro Features/        # Advanced features examples
│   │   └── ESP32-S3 Progress Bar/  # Progress bar implementation
│   ├── ThreadSafe/          # Thread safety examples
│   │   └── ESP32-S3 Thread Safety/ # Multi-threading examples
│   └── Desktop/             # Desktop application examples
├── CMakeLists.txt           # Multi-platform CMake configuration
├── library.json             # PlatformIO library manifest
├── library.properties       # Arduino library properties
├── idf_component.yml        # ESP-IDF component configuration
└── README.md
```

## 🛣️ Roadmap & Upcoming Features

ChronoLog is actively developed with exciting features planned for future releases:

### 🔜 Next Major Features
- **📡 Remote Logging**: Custom user channels via callbacks to route logs anywhere you need
- **💾 Flash File Logging**: Log directly to flash memory with retrieval capabilities  
- **⚡ Memory & Speed Optimization**: Enhanced performance for resource-constrained systems
- **🔧 Robust 8-bit AVR Support**: Full compatibility with Arduino Uno, Nano, and other 8-bit platforms

### 🎯 Advanced Features In My Mind
- **🌐 Network Logging**: UDP/TCP log streaming for IoT applications
- **📊 Log Analytics**: Built-in log filtering and analysis tools
- **🔒 Encrypted Logging**: Secure log transmission and storage
- **📱 Mobile App Integration**: Real-time log monitoring via smartphone apps

### 📈 Performance Improvements
- **🚀 Zero-copy Logging**: Minimize memory allocations for high-frequency logging
- **⚙️ Compile-time Optimization**: Further reduction in code size and RAM usage
- **🔄 Circular Buffer Support**: Efficient log rotation for continuous operation

**Want to influence the roadmap?** Share your ideas through [GitHub Issues](https://github.com/Hamas888/ChronoLog/issues) or reach out via [Patreon](https://patreon.com/hamas888) for direct feature discussions!

## 💖 Support & Motivation

ChronoLog is developed in my spare time with passion for the embedded community. Your support helps motivate continued development of this and other upcoming sensor libraries and embedded tools.

### 🎯 Support the Project
- **⭐ Star this repository** - It means a lot and helps others discover ChronoLog
- **🔄 Patreon** - [Monthly support for ongoing development](https://patreon.com/hamas888)

### 💬 Get Help & Discuss
- **🐛 Issues**: Create a [GitHub Issue](https://github.com/Hamas888/ChronoLog/issues) for bugs or feature requests
- **💭 Direct Help**: Reach out on [Patreon](https://patreon.com/hamas888) for direct support and discussions
- **📧 Email**: Contact me at hamasaeed@gmail.com for collaboration or questions

Your support enables me to:
- 🔧 Maintain and improve ChronoLog
- 📡 Develop upcoming sensor libraries 
- 🚀 Create more embedded development tools
- 📚 Provide community support and documentation

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
