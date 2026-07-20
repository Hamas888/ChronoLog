# ChronoLog — ESP32 Examples

ChronoLog supports **3 build systems** on ESP32. Each has the same 4 examples: Logging, ProgressBar, RemoteLogging, and ThreadSafety.

---

## 1. PlatformIO (Recommended)

Single `platformio.ini` with 8 environments. Pulls ChronoLog from the registry.

```ini
[env:arduino-logging]
platform = espressif32
board = esp32-s3-devkitm-1
framework = arduino
lib_deps = ChronoLog
```

```bash
pio run -e arduino-logging
```

See [PlatformIO/](PlatformIO/) for all environments.

## 2. ESP-IDF Native

Each example is a standalone `idf.py` project. ChronoLog is pulled via the component manager:

```bash
cd ESP-IDF/Logging
idf.py add-dependency "hamas888/chronolog^1.1.5"
idf.py set-target esp32s3
idf.py build flash monitor
```

The `main/idf_component.yml` already declares the dependency — `idf.py build` fetches it automatically. See [ESP-IDF/](ESP-IDF/) for all examples.

## 3. Arduino IDE

Open any `.ino` sketch from [Arduino/](Arduino/) in the Arduino IDE.

Install ChronoLog: **Sketch → Include Library → Manage Libraries → search "ChronoLog" → Install**.

Then select your board and upload.

---

## Example Overview

| Example | What It Shows |
|---|---|
| [Logging](PlatformIO/src/arduino_Logging.cpp) | Basic log levels (debug, info, warn, error, fatal) |
| [ProgressBar](PlatformIO/src/arduino_ProgressBar.cpp) | In-place progress bar with `logger.progress()` |
| [RemoteLogging](PlatformIO/src/arduino_RemoteLogging.cpp) | TCP streaming logs to remote clients |
| [ThreadSafety](PlatformIO/src/arduino_ThreadSafety.cpp) | Concurrent logging from multiple FreeRTOS tasks |
