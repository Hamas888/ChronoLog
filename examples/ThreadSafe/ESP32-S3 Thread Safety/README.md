# ChronoLog Thread-Safe Logging Example for ESP32-S3

This example demonstrates the thread-safe capabilities of ChronoLog when multiple FreeRTOS tasks log messages concurrently on a dual-core ESP32-S3 microcontroller.

## 🔒 Thread Safety Features Demonstrated

### What This Example Shows:
1. **Multi-task Concurrent Logging**: Multiple tasks running on different cores simultaneously writing to the same logger
2. **Mutex Protection**: ChronoLog's built-in mutex prevents race conditions and log message corruption
3. **Cross-Core Safety**: Tasks on Core 0 and Core 1 safely share the same logger instance
4. **Mixed Priority Tasks**: Different task priorities accessing the logger without conflicts
5. **Multiple Log Levels**: DEBUG, INFO, WARNING, and ERROR messages from different tasks

### Without Thread Safety:
Without `CHRONOLOG_THREAD_SAFE = 1`, you would see:
- Corrupted/garbled log messages
- Incomplete log entries
- Race conditions between cores
- Unpredictable output formatting

### With Thread Safety (This Example):
- Clean, complete log messages from all tasks
- Proper message ordering and formatting
- No corruption or race conditions
- Reliable logging across multiple cores

## 🏗️ Architecture

### Tasks Created:
1. **Sensor Task** (Core 0, Priority 2):
   - Simulates sensor readings every 800ms
   - Logs sensor values with thresholds
   - Demonstrates DEBUG, INFO, WARNING, and ERROR levels

2. **Network Task** (Core 1, Priority 2):
   - Simulates network packet processing every 600ms
   - Logs network events and error conditions
   - Shows concurrent logging from different core

3. **System Task** (Core 0, Priority 1):
   - Simulates system monitoring every 1200ms
   - Logs system health and maintenance events
   - Runs on same core as Sensor Task (different priorities)

4. **Main Loop** (Core 1, Arduino Loop):
   - Logs every 3 seconds with system status
   - Shows memory usage and task states
   - Demonstrates main thread logging alongside FreeRTOS tasks

## 🔧 Configuration

```cpp
#define CHRONOLOG_THREAD_SAFE 1  // Enable thread safety
```

This enables:
- FreeRTOS mutex creation and management
- Automatic mutex locking/unlocking around log operations
- Thread-safe buffer management
- Cross-core synchronization

## 📊 Expected Output

You should see interleaved log messages like:

```
[INFO ] [ThreadSafeLogger] === ESP32-S3 Thread-Safe Logging Example ===
[INFO ] [ThreadSafeLogger] ChronoLog Thread Safety: ENABLED
[INFO ] [ThreadSafeLogger] Available cores: 2
[INFO ] [ThreadSafeLogger] Task 1: Sensor monitoring task started on core 0
[INFO ] [ThreadSafeLogger] Task 2: Network monitoring task started on core 1
[INFO ] [ThreadSafeLogger] Task 3: System monitoring task started on core 0
[DEBUG] [ThreadSafeLogger] Task 1: Reading sensor... Value: 7
[DEBUG] [ThreadSafeLogger] Task 2: Processing network packet #1
[INFO ] [ThreadSafeLogger] Task 1: Sensor data logged successfully
[INFO ] [ThreadSafeLogger] Task 2: Network packet 1 processed successfully
[DEBUG] [ThreadSafeLogger] Task 3: System event #1 triggered
[INFO ] [ThreadSafeLogger] Main Loop: Iteration 1 - All tasks running smoothly
```

## 🎯 Key Learning Points

1. **Seamless Integration**: No special code needed in tasks - just use the logger normally
2. **Automatic Protection**: ChronoLog handles all mutex operations internally
3. **Performance**: Minimal overhead while ensuring thread safety
4. **Scalability**: Works with any number of tasks and cores
5. **Reliability**: Guarantees message integrity in multi-threaded environments

## 🔬 Testing Thread Safety

To verify thread safety is working:

1. **Build and Upload**: Flash this example to your ESP32-S3
2. **Monitor Serial**: Watch the serial output at 115200 baud
3. **Observe**: Look for clean, non-corrupted messages from all tasks
4. **Compare**: Try disabling thread safety (`#define CHRONOLOG_THREAD_SAFE 0`) to see the difference

## 🛠️ Hardware Requirements

- ESP32-S3 development board (dual-core)
- USB cable for programming and serial monitoring
- Serial terminal (Arduino IDE Serial Monitor, PlatformIO Serial Monitor, etc.)

## 📝 Build Instructions

```bash
# Using PlatformIO
pio run -t upload
pio device monitor

# Or using Arduino IDE
# Open main.cpp, compile and upload
```

## 🎪 Advanced Usage

This example can be extended to demonstrate:
- File logging with thread safety
- Network logging from multiple tasks
- Real-time data logging from sensors
- Multi-core data processing with logging
- Error handling and recovery logging

## 💡 Tips

- Each task logs at different intervals to show true concurrency
- Tasks run on different cores to stress-test thread safety
- Mixed priorities ensure realistic real-world scenarios
- Memory monitoring shows system stability under concurrent logging

## 🔗 Related Examples

- Basic ChronoLog usage
- Time synchronization examples
- Advanced formatting features
- Custom output targets