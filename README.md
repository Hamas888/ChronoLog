# ChronoLog 🕒

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Version](https://img.shields.io/badge/Version-1.2.0-green.svg)](https://github.com/Hamas888/ChronoLog)
[![Platform](https://img.shields.io/badge/Platform-ARDUINO%20|%20ESP32%20|%20STM32%20|%20nRF52%20|%20Linux%20|%20Windows%20|%20MacOS%20-orange.svg)](https://github.com/Hamas888/ChronoLog)
[![Desktop CI](https://github.com/Hamas888/ChronoLog/actions/workflows/desktop-ci.yml/badge.svg)](https://github.com/Hamas888/ChronoLog/actions/workflows/desktop-ci.yml)
[![Desktop Examples CI](https://github.com/Hamas888/ChronoLog/actions/workflows/desktop-examples-ci.yml/badge.svg)](https://github.com/Hamas888/ChronoLog/actions/workflows/desktop-examples-ci.yml)
[![PlatformIO CI](https://github.com/Hamas888/ChronoLog/actions/workflows/platformio-ci.yml/badge.svg)](https://github.com/Hamas888/ChronoLog/actions/workflows/platformio-ci.yml)

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
- [📜 Changelog](#-changelog)
- [📊 Memory Footprint](#-memory-footprint)
- [⚖️ Comparison with Vendor Loggers](#️-comparison-with-vendor-loggers)
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
- **📦 Zephyr Module**: Discoverable via `west.yml` — just add to your manifest
- **📚 Per-Platform READMEs**: Step-by-step guides for each build system in `examples/`
- **📊 Multiple Log Levels**: DEBUG, INFO, WARN, ERROR, FATAL with runtime level control
- **🎛️ Module-based Logging**: Create separate loggers for different modules with individual log levels
- **📈 Progress Bar Support**: Built-in progress tracking with visual progress bars (requires `CHRONOLOG_PRO_FEATURES`)
- **📊 Graph Plotter**: ASCII graph plotting of numeric series — live sparklines and multi-row window charts (requires `CHRONOLOG_PRO_FEATURES`)
- **💻 Desktop Support**: Full compatibility with Linux, Windows, and macOS
- **💾 Memory Efficient**: Minimal per-instance overhead (8 bytes RAM) with ~3 KB flash footprint
- **🚀 Zero Configuration**: Works out-of-the-box on supported platforms
- **⚙️ Conditional Compilation**: Enable only the features you need to optimize for your platform
- **📦 Official Package Managers**: Available on Arduino IDE Library Manager, PlatformIO, and ESP-IDF Component Manager
- **🧵 True Cross-Platform Thread Safety**: Mutex abstraction across 5 RTOS/threading models

## 🎯 Supported Platforms

> **Note on timestamps**: On embedded targets without RTC/sync, timestamps are millisecond-uptime counters formatted as `HH:MM:SS` (wrapping at 24h). ESP platforms with NTP-synced system time display wall-clock time. System-time-based timestamps for STM32 and nRF are planned.
>
> **Note on colors**: ANSI escape sequences are supported on all platforms ✅ but are **disabled by default** (`CHRONOLOG_COLOR_ENABLE=0`) to avoid overhead on terminals that don't support them. Enable via `#define CHRONOLOG_COLOR_ENABLE 1` or Kconfig.

| Platform | Framework | RTOS/Thread | Timestamp Source | Task Names | Output Method | Remote TCP | Colors |
|----------|-----------|-------------|-----------------|------------|--------------|------------|--------|
| **ESP32** | Arduino | ✅ FreeRTOS | ⏱️ gettimeofday (ms)¹ | ✅ pcTaskGetName | Serial.printf | ❌ | ✅ ANSI |
| **ESP32** | ESP-IDF | ✅ FreeRTOS | ⏱️ gettimeofday (ms)¹ | ✅ pcTaskGetName | printf | ✅ lwIP | ✅ ANSI |
| **STM32** | HAL + FreeRTOS | ✅ CMSIS-OS | ⏱️ HAL_GetTick (ms uptime)² | ✅ pcTaskGetName | UART | ❌ | ✅ ANSI |
| **STM32** | HAL (bare metal) | ❌ | ⏱️ HAL_GetTick (ms uptime)² | ❌ "MainTask" | UART | ❌ | ✅ ANSI |
| **nRF52/nRF54** | nRF Connect SDK | ✅ Zephyr | ⏱️ k_uptime_get (ms uptime)² | ✅ k_thread_name_get | printf | ❌ | ✅ ANSI |
| **Arduino Uno Q** | Arduino (Zephyr) | ✅ Zephyr | ⏱️ k_uptime_get (ms uptime) | ✅ thread map | Monitor | ❌ | ✅ ANSI |
| **Arduino AVR** | Arduino (plain) | ❌ | ❌ (not yet impl.) | ❌ "MainTask" | Serial.print | ❌ | ✅ ANSI |
| **Linux** | Native | ✅ pthread | ✅ system_clock (s) | ✅ thread_local | printf | ✅ POSIX | ✅ ANSI |
| **Windows** | Native | ✅ std::thread | ✅ system_clock (s) | ✅ thread_local | printf | ✅ POSIX | ✅ ANSI |
| **macOS** | Native | ✅ pthread | ✅ system_clock (s) | ✅ thread_local | printf | ✅ POSIX | ✅ ANSI |

¹ Local wall-clock time **if** NTP/system-time is configured; otherwise shows boot epoch (00:00:00).
² Uptime counter — wraps at 24h. System-time (RTC) support is planned for a future release.

## 📦 Installation

ChronoLog is officially available on multiple package managers! Choose your preferred installation method:

### 🎯 Quick Installation

**Option 1: Arduino IDE Library Manager** ⭐ (Easiest for Arduino)
- Open Arduino IDE → Sketch → Include Library → Manage Libraries
- Search for "ChronoLog"
- Click Install

**Option 2: PlatformIO Library Manager** ⭐ (Recommended for ESP32/ESP-IDF)
- Open PlatformIO → Libraries → Search for "ChronoLog" → Install
- Or manually add to `lib_deps` in `platformio.ini`

**Option 3: ESP-IDF Component Manager** ⭐ (For ESP-IDF Projects)
- ESP-IDF 5.0+ automatically finds ChronoLog in the component registry
- Or manually add to your project's dependency manifest

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

Add ChronoLog to your `west.yml`:

```yaml
manifest:
  projects:
    - name: chronolog
      url: https://github.com/Hamas888/ChronoLog
      revision: v1.2.0
      path: modules/lib/chronolog
  self:
    path: app
```

Or without `west`, set `ZEPHYR_MODULES` before `find_package(Zephyr)`:

```bash
cmake -B build -DZEPHYR_MODULES=/path/to/ChronoLog
```

ChronoLog provides a `zephyr/module.yml` so it's auto-discovered and linked. Just `#include "ChronoLog.h"` in your source.

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

### Graph Plotter Example

```cpp
#include "ChronoLog.h"

// Enable pro features in your build configuration
// #define CHRONOLOG_PRO_FEATURES 1

ChronoLogger logger("Plotter", CHRONOLOG_LEVEL_PRO_FEATURES);

void plotLiveData() {
    // Live sparkline: push one sample at a time (updates in place with '\r')
    logger.plot("temp", 24.5f);
    logger.plot("temp", 25.1f);
    logger.plot("temp", 23.8f);
}

void plotBatchData() {
    // Batch plot: push many samples at once, renders a full window chart
    float accel[8] = { 1.0f, 2.0f, 3.0f, 2.0f, 5.0f, 4.0f, 3.0f, 6.0f };
    logger.plot("accel", accel, 8);
}

void reRenderWindows() {
    logger.plotWindow("temp");   // Re-render one series as a row chart
    logger.plotWindow();         // Re-render all registered series
}
```

> **Note**: Plots require `CHRONOLOG_PRO_FEATURES` to be enabled and the logger level set to `CHRONOLOG_LEVEL_PRO_FEATURES` or higher. Plot output is terminal-only (not sent over the remote TCP channel). Sample buffers are compile-time configurable via `CHRONOLOG_PLOT_WINDOW` / `CHRONOLOG_PLOT_SERIES` / `CHRONOLOG_PLOT_ROWS`.

### Remote Logging Example

```cpp
#include "ChronoLog.h"

// Enable remote logging in your build configuration
// #define CHRONOLOG_REMOTE_ENABLE 1

ChronoLogger logger("RemoteApp", CHRONOLOG_LEVEL_INFO);

void setupRemoteLogging() {
    #if CHRONOLOG_REMOTE_ENABLE
        // Start the remote logging server on port 9999
        ChronoLogRemote* remoteLogger = ChronoLogRemote::getInstance();
        bool started = remoteLogger->start(9999);
        
        if (started) {
            logger.info("Remote logging started on port 9999");
        } else {
            logger.error("Failed to start remote logging server");
        }
    #else
        logger.warn("Remote logging is disabled. Enable with CHRONOLOG_REMOTE_ENABLE=1");
    #endif
}

void applicationLoop() {
    logger.info("Application started - logs are streamed to connected clients");
    logger.debug("This message is sent to all connected remote clients");
    logger.error("Error logs are also transmitted to remote monitors");
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
Color coding is applied to log levels when enabled (`#define CHRONOLOG_COLOR_ENABLE 1` or via Kconfig):
- 🔵 **DEBUG**: Blue (Italic)
- 🟢 **INFO**: Green (Italic)
- 🟡 **WARN**: Yellow (Italic)
- 🔴 **ERROR**: Red (Italic)
- 🟣 **FATAL**: Magenta (Italic)
- 🟠 **PROGRESS** (In-progress): Orange (Italic)
- 🔵 **PROGRESS** (Complete): Cyan (Italic)

> **Note**: Colors are **disabled by default** (`CHRONOLOG_COLOR_ENABLE=0`) to avoid ANSI escape garbage on terminals that don't support them. Enable explicitly for ANSI-capable serial monitors (most modern terminals support them, including Arduino Serial Monitor, PuTTY, and IDE serial consoles).

## ⚙️ Configuration

### Compile-Time Feature Configuration

ChronoLog allows you to enable or disable features at compile time to optimize for your specific use case:

```cpp
// === Core Logging Configuration ===
#define CHRONOLOG_MODE 1                    // Set to 0 to disable all logging
#define CHRONOLOG_BUFFER_LEN 256            // Buffer size for log messages (default: 256)
#define CHRONOLOG_DEFAULT_LEVEL CHRONOLOG_LEVEL_DEBUG  // Default log level

// === Feature Control ===
#define CHRONOLOG_COLOR_ENABLE 1            // Enable colored output (1 = enabled, 0 = disabled)
#define CHRONOLOG_THREAD_SAFE 1             // Enable thread-safe operations (1 = enabled, 0 = disabled)
#define CHRONOLOG_PRO_FEATURES 1            // Enable Pro features like progress bars (1 = enabled, 0 = disabled)
#define CHRONOLOG_REMOTE_ENABLE 0           // Enable remote logging via TCP (1 = enabled, 0 = disabled)

// === Graph Plotter Configuration (Pro features only) ===
#define CHRONOLOG_PLOT_WINDOW 64            // Samples retained per plot series (ring buffer)
#define CHRONOLOG_PLOT_SERIES 4             // Maximum number of named plot series tracked simultaneously
#define CHRONOLOG_PLOT_ROWS 5               // Chart height (rows) used by plotWindow()
// #define CHRONOLOG_PLOT_BLOCKS " .:-=+*#%@"  // Sparkline glyphs; override for ASCII-only terminals (default: ▁▂▃▄▅▆▇█)

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
#define CHRONOLOG_REMOTE_ENABLE 0           // Disable remote logging
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

**Note**: `CHRONOLOG_LEVEL_PRO_FEATURES` is only available when `CHRONOLOG_PRO_FEATURES` is defined as 1. This conditional level enables advanced features like progress bars and graph plotting while maintaining backward compatibility.

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
- Output via `printf` (routes through Zephyr's console backend)
- Automatic thread name detection

### Arduino Uno Q (Zephyr-based)
- No additional setup required
- Output via `Monitor.print()` (Arduino_RouterBridge bridge)
- Automatic thread name detection via thread-map
- Timestamps from Zephyr `k_uptime_get()` (ms uptime)

### Arduino AVR (8-bit, plain Arduino)
- No additional setup required
- Output via `Serial.print()` at the configured baud rate
- ⚠️ Timestamp output not yet implemented for this platform — `time_buf` will show uninitialized data
- Task name always shows "MainTask" (no RTOS)
- Colors supported but **disabled by default** — set `CHRONOLOG_COLOR_ENABLE=1` to enable
- For memory-constrained targets, reduce `CHRONOLOG_BUFFER_LEN` (default 256) and disable `CHRONOLOG_THREAD_SAFE`

### Desktop Platforms (Linux, Windows, macOS)
- **Thread Safety**: Automatic mutex-based synchronization for multi-threaded applications
- **Progress Bars**: Full support for visual progress tracking
- **Graph Plotter**: Full support for ASCII sparklines and window charts
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

**Graph plots not working / garbled**
- Verify `#define CHRONOLOG_PRO_FEATURES 1` is set
- Ensure logger level is `CHRONOLOG_LEVEL_PRO_FEATURES` or higher
- Plots are terminal-only — they are not sent over the remote TCP channel
- On ASCII-only terminals, set `CHRONOLOG_PLOT_BLOCKS` to a plain string like `" .:-=+*#%@"`
- For RAM-constrained targets, reduce `CHRONOLOG_PLOT_WINDOW` and `CHRONOLOG_PLOT_SERIES`

**Compilation errors on platform detection**
- Platform detection is automatic; avoid manually defining platform macros
- For custom platforms, check the header file for supported detection patterns

**Thread safety issues**
- Thread safety is enabled by default (`CHRONOLOG_THREAD_SAFE 1`)
- On custom RTOS implementations, mutex initialization may need adjustment

## 📁 Repository Structure

```
ChronoLog/
├── src/
│   ├── ChronoLog.h                 # Main header — public API
│   ├── ChronoLog.cpp               # Core implementation
│   ├── ChronoLogRemote.h           # Remote TCP logging — declarations
│   ├── ChronoLogRemote.cpp         # Remote TCP logging — implementation
│   └── ChronoLogUnoQ.h            # Arduino Uno Q (Zephyr) shims
├── include/
│   └── ChronoLog.h                 # Facade header (forwards to src/)
├── examples/
│   ├── Desktop/                    # Native PC (Linux/macOS/Windows) — CMake + FetchContent
│   ├── ESP/                        # ESP32 — 3 build systems
│   │   ├── Arduino/               #   Arduino IDE (.ino sketches)
│   │   ├── ESP-IDF/               #   ESP-IDF native (idf.py)
│   │   └── PlatformIO/            #   PlatformIO (8 envs: Arduino + ESP-IDF)
│   ├── NRF/                        # nRF52 / Zephyr — west + prj.conf
│   ├── STM/                        # STM32 — CubeMX + CMake (Nucleo-L010RB)
│   ├── UnoQ/                       # Arduino Uno Q (Zephyr-based)
│   └── CMakeLists.txt              # Unified test runner
├── zephyr/
│   └── module.yml                  # Zephyr module descriptor
├── tests/                          # Desktop test harness (CTest)
├── .github/workflows/              # CI: Desktop, PlatformIO, Docker image
├── CMakeLists.txt                  # Multi-platform CMake configuration
├── Kconfig                         # ESP-IDF / Zephyr Kconfig menu
├── Dockerfile                      # CI Docker image (PlatformIO + CMake)
├── library.json                    # PlatformIO library manifest
├── library.properties              # Arduino library properties
└── idf_component.yml               # ESP-IDF component manifest
```

## 📜 Changelog

### v1.2.0 (Current)

**Bugs Fixed**

- **ESP32 double-checked locking fix**: `portMUX_TYPE` in constructor changed from local to `static` — concurrent `xSemaphoreCreateMutex()` calls on dual-core could leak the first semaphore.
- **Macro leak fix**: `#define vsnprintf` / `#define strncat` moved from `ChronoLogUnoQ.h` into `ChronoLog.cpp` — these were replacing standard library functions in any translation unit that included ChronoLog.
- **Progress bar newline race**: `progress()` now holds the mutex across the entire bar + newline sequence instead of relying on `printProgress()`'s internal lock.
- **Remote log ordering**: TCP send moved before `threadSafeUnlock()` in `print()` — serial and remote output are now emitted under the same lock.
- **Header guard conflict**: `include/ChronoLog.h` and `src/ChronoLog.h` both used `CHRONOLOG_H` — the facade blocked the real header. Changed facade to `CHRONOLOG_FACADE_H`.

**New Features**

- **Zephyr module support**: Added `zephyr/module.yml` — ChronoLog is now auto-discoverable via `west.yml` or `ZEPHYR_MODULES`.
- **CI/CD pipeline**: Desktop CI, Desktop Examples CI, PlatformIO CI (8 environments), and Docker image build workflows. CI Docker image pre-caches all PlatformIO toolchains.
- **Per-platform example READMEs**: Step-by-step build guides for ESP (3 build systems), STM32, and NRF examples.
- **Desktop examples**: 4 standalone CMake examples with CTest validation — Logging, ProgressBar, RemoteLogging, ThreadSafety.
- **STM32 examples**: Full CubeMX projects with `.ioc` files for Nucleo-L010RB — Logging, ProgressBar, ThreadSafety.

**Design Improvements**

- **Buffer size**: `CHRONOLOG_BUFFER_LEN` default increased from 100 → 256.
- **ChronoLogRemote**: Implementation moved from header-only to `ChronoLogRemote.cpp` (~600 B saving per translation unit).
- **Safer defaults**: `#warning` emitted when `ChronoLogRemote.h` is included without `CHRONOLOG_REMOTE_ENABLE`.
- **Timestamp computed once**: `printInfo()` receives pre-computed `time_buf` and `taskName` — remote no longer re-calls time/task functions.

**Example Reorganization**

- Examples restructured by platform: `examples/ESP/` (ESP32), `examples/NRF/` (nRF52/Zephyr), `examples/STM/` (STM32), `examples/UnoQ/` (Arduino Uno Q).
- PlatformIO examples simplified: single `platformio.ini` with 8 environments, `build_flags` for feature defines so library compilation matches example source.

**Nits**

- Include guard typo: `CHRNOLOG_UNO_Q_H` → `CHRONOLOG_UNO_Q_H`
- Level label: `"WARNING"` → `"WARN"` for 5-char alignment

---

### v1.1.5 (Previous)

- Initial stable release with cross-platform support (Arduino, ESP-IDF, STM32 HAL, Zephyr, Desktop)
- Thread safety across 5 RTOS/threading models
- Progress bar with color-coded completion indicators
- Remote TCP logging (ESP-IDF, Desktop)
- Arduino Uno Q (Zephyr) support

---

## 📊 Memory Footprint

### Per-instance RAM

| Member | Size | Notes |
|--------|------|-------|
| `name` (pointer) | 4 B | Points to user-provided string (not copied) |
| `chronoLogLevel` (enum) | 4 B | Per-instance level filtering |
| `uartHandler` | 4 B | STM32 HAL only |
| **Total per instance** | **8 B** (12 B on STM32) | |

### Global / static RAM

| Object | Platform | Size |
|--------|----------|------|
| `chronoLogMutex` | Desktop (`std::mutex`) | ~8 B |
| `chronoLogMutex` | STM32 FreeRTOS (handle) | 4 B |
| `chronoLogMutex` | ESP-IDF/ESP (handle) | 4 B |
| `chronoLogMutex` | Zephyr/Uno Q (`k_mutex`) | ~16 B |
| `threadsMap[10]` | Uno Q only | 80 B |
| `plotSeries[4]` (plot registry) | Pro features only | ~1.1 KB (`4 × (16 B name + 64 × 4 B samples)`) |

### Stack usage (hot path)

| Call path | Stack | Main consumers |
|-----------|-------|----------------|
| `print()` → `printInfo()` | ~300 B | `msg_buf[256]` + `time_buf[16]` |
| `print()` + remote | ~820 B | `msg_buf[256]` + `full_buf[512]` |
| `progress()` → `printProgress()` | ~120 B | `prog_buf[100]` + `time_buf[16]` |
| `plot()` → `renderSparkline()` | ~330 B | `line_buf[4 + 64×4 + 64]` |
| `plot()` / `plotWindow()` → `renderWindowChart()` | ~320 B | `line_buf[64 + 64×4]` |

### Estimated flash (`.text`)

| Component | Size |
|-----------|------|
| 5 log level wrappers | ~250 B |
| `print()` + format + truncation | ~500 B |
| `printInfo()` header formatter | ~350 B |
| `printProgress()` bar renderer | ~400 B |
| `renderSparkline()` + `renderWindowChart()` | ~500 B |
| `getTimeStamp()` + `getCurrentTaskName()` | ~450 B |
| `threadSafeLock()` / `threadSafeUnlock()` | ~200 B |
| Remote TCP impl | ~600 B |
| Uno Q shims | ~300 B |
| **Total** | **~3.0–3.5 KB** |

> Stack is allocated at call-time, not per-instance. With `CHRONOLOG_MODE=0`, all code compiles to zero bytes.

### Multiple instances

Each additional `ChronoLogger` costs **8 bytes RAM** and **zero extra flash** — all instances share code and the global mutex.

| Scenario | Instance RAM | Global RAM | Total RAM | Flash |
|----------|-------------|------------|-----------|-------|
| 1 logger | 8 B | 4–16 B | **12–24 B** | ~3 KB |
| 4 loggers | 32 B | 4–16 B | **36–48 B** | ~3 KB |
| 10 loggers | 80 B | 4–16 B | **84–96 B** | ~3 KB |

## ⚖️ Comparison with Vendor Loggers

| Feature | **ChronoLog** | **ESP-IDF `esp_log`** | **STM32 HAL `printf`** | **Zephyr logging** | **nRF5 SDK** |
|---------|:------------:|:--------------------:|:---------------------:|:------------------:|:-----------:|
| **RAM per instance** | 8 B | ~4 B (tag ptr) | 0 (global) | ~8 B | ~4 B |
| **Flash footprint** | ~3 KB | ~5–10 KB | ~0.5 KB¹ | ~2–4 KB | ~4–6 KB |
| **Per-instance log level** | ✅ | ❌ Global only | ❌ | ✅ | ❌ Global |
| **Timestamp (built-in)** | ✅ All platforms | ✅ via system | ❌ Manual | ✅ | ✅ |
| **Task/thread name** | ✅ | ❌ | ❌ | ✅ | ❌ |
| **Color output** | ✅ ANSI | ❌ | ❌ | ❌ | ❌ |
| **Progress bar** | ✅ Built-in | ❌ | ❌ | ❌ | ❌ |
| **Remote TCP streaming** | ✅ Built-in | ❌ | ❌ | ❌ (net backend exists) | ❌ |
| **Cross-platform** | ✅ 7+ platforms | ❌ ESP only | ❌ STM only | ❌ Zephyr only | ❌ nRF only |
| **Thread-safe** | ✅ All RTOS | ✅ | ❌ (user does it) | ✅ | ✅ w/ FreeRTOS |
| **Zero-cost disable** | ✅ `MODE=0` | ✅ Kconfig | N/A | ✅ Kconfig | ✅ sdk_config |
| **Compile-time filtering** | ✅ Per instance | ✅ Via tag | ❌ | ✅ Via level | ✅ Via level |

¹ STM32 HAL `printf` redirect is tiny but provides **none** of: levels, timestamps, modules, task names, or thread safety.

**Key takeaway**: ChronoLog matches or exceeds vendor loggers in every feature category while maintaining a **smaller flash footprint** than most and costing **only 8 extra bytes of RAM per instance**. The true differentiator is **cross-platform portability** — write your logging code once, compile it on any target.

## 🧪 Automated Tests

A native desktop test harness is included under `tests/` to validate the library build and basic desktop logging output.

### Run locally with CMake

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

This now also includes CTest wrappers for desktop example executables, so the examples can be run automatically as part of the same test suite rather than only through GitHub Actions.

### GitHub Actions

A workflow is available at `.github/workflows/desktop-ci.yml` to build the library and run the desktop test automatically on `push`, `pull_request`, or via manual dispatch.

A second workflow at `.github/workflows/desktop-examples-ci.yml` builds the desktop example binaries and runs each one briefly to confirm the runtime behavior before termination:
- `DesktopLogging`
- `DesktopProgressBar`
- `DesktopThreadSafety`
- `DesktopRemoteLogging`

### Trigger workflows locally

If you have GitHub CLI installed:

```bash
gh workflow run desktop-ci.yml
gh workflow run desktop-examples-ci.yml
```

If you want a local simulator for GitHub Actions:

```bash
act workflow_dispatch -W .github/workflows/desktop-ci.yml
act workflow_dispatch -W .github/workflows/desktop-examples-ci.yml
```

> Note: `act` requires Docker and may need a Linux-compatible image.

## 🛣️ Roadmap & Upcoming Features

### 🔜 v1.3.0 — Configurable Output Backends
- **Plugin-style backend system**: `logger.addBackend(Serial)`, `logger.addBackend(RTT)`, `logger.addBackend(FileSD)` — swap output destinations at runtime
- **STM32CubeMX `.pack`**: CMSIS-Pack for STM32CubeMX registry — install ChronoLog from within CubeMX
- **STM32CubeMX integration**: Documented `.ioc` workflow with UART + FreeRTOS configuration
- **Zephyr module registry**: List ChronoLog in the official Zephyr module registry

### 🎯 Future Ideas
- **Configurable output backends**: File I/O (SD card), RTT, semihosting, USB CDC
- **Async / non-blocking mode**: Queue-based logging for ISR-safe and high-throughput paths
- **📊 Log Filtering**: Built-in pattern matching and log analytics
- **🔒 Encrypted Logging**: Secure log transmission and storage

### 📈 Performance
- **Zero-copy Logging**: Minimize allocations for high-frequency logging
- **Circular Buffer Support**: Efficient log rotation for continuous operation

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
