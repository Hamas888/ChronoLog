#define CHRONOLOG_THREAD_SAFE           1           // Enabled thread-safe logging
#define CHRONOLOG_COLOR_ENABLE          1           // 0 = Disable colors for ANSII-incompatible terminals

#include "../../../include/ChronoLog.h"
#include <thread>
#include <chrono>
#include <atomic>
#include <random>

ChronoLogger logger("ThreadSafeLogger", CHRONOLOG_LEVEL_DEBUG);

// Simulated sensor data
std::atomic<int> sensorValue(0);
std::atomic<int> networkPackets(0);
std::atomic<int> systemEvents(0);

// Helper for random numbers
int getRandom(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

// Task 1: Sensor Data Logger
void sensorTask() {
    int taskId = 1;
    logger.info("Task %d: Sensor monitoring task started", taskId);
    
    while (true) {
        sensorValue += getRandom(1, 10);
        
        // Multiple log levels to show thread safety across different message types
        logger.debug("Task %d: Reading sensor... Value: %d", taskId, sensorValue.load());
        
        if (sensorValue > 50) {
            logger.warn("Task %d: Sensor value high: %d - checking thresholds", taskId, sensorValue.load());
        }
        
        if (sensorValue > 80) {
            logger.error("Task %d: CRITICAL! Sensor value too high: %d", taskId, sensorValue.load());
            sensorValue = 0; // Reset for demo
        }
        
        logger.info("Task %d: Sensor data logged successfully", taskId);
        std::this_thread::sleep_for(std::chrono::milliseconds(800)); // 800ms delay
    }
}

// Task 2: Network Activity Logger
void networkTask() {
    int taskId = 2;
    logger.info("Task %d: Network monitoring task started", taskId);
    
    while (true) {
        networkPackets += getRandom(1, 5);
        
        logger.debug("Task %d: Processing network packet #%d", taskId, networkPackets.load());
        
        // Simulate different network events
        if (networkPackets % 7 == 0) {
            logger.warn("Task %d: Network latency detected on packet %d", taskId, networkPackets.load());
        }
        
        if (networkPackets % 15 == 0) {
            logger.error("Task %d: Network timeout on packet %d - retrying", taskId, networkPackets.load());
        }
        
        logger.info("Task %d: Network packet %d processed successfully", taskId, networkPackets.load());
        std::this_thread::sleep_for(std::chrono::milliseconds(600)); // 600ms delay
    }
}

// Task 3: System Events Logger
void systemTask() {
    int taskId = 3;
    logger.info("Task %d: System monitoring task started", taskId);
    
    while (true) {
        systemEvents++;
        
        logger.debug("Task %d: System event #%d triggered", taskId, systemEvents.load());
        
        // Simulate system events
        if (systemEvents % 5 == 0) {
            logger.info("Task %d: Memory usage check - Event %d", taskId, systemEvents.load());
        }
        
        if (systemEvents % 10 == 0) {
            logger.warn("Task %d: Periodic system health check - Event %d", taskId, systemEvents.load());
        }
        
        if (systemEvents % 20 == 0) {
            logger.error("Task %d: System maintenance required - Event %d", taskId, systemEvents.load());
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1200)); // 1200ms delay
    }
}

int main() {
    logger.debug("Desktop Thread Safety Example Starting...");
    logger.info("Logger initialized successfully at DEBUG level with Thread Safety ENABLED");

    std::thread t1(sensorTask);
    std::thread t2(networkTask);
    std::thread t3(systemTask);

    t1.join();
    t2.join();
    t3.join();

    return 0;
}
