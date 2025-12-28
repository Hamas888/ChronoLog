#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/random/random.h>

#define CHRONOLOG_THREAD_SAFE           1           // Enabled thread-safe logging
#define CHRONOLOG_COLOR_ENABLE          1           // 0 = Disable colors for ANSII-incompatible terminals

#include "ChronoLog.h"

// Loggers
static ChronoLogger logger("System", CHRONOLOG_LEVEL_DEBUG);
static ChronoLogger sensorLogger("Sensors", CHRONOLOG_LEVEL_DEBUG);
static ChronoLogger networkLogger("Network", CHRONOLOG_LEVEL_DEBUG);

// Simulated data
volatile int sensorValue    = 0;
volatile int networkPackets = 0;
volatile int systemEvents   = 0;

// Thread 1: Sensor Data Logger
void sensor_thread_entry(void *, void *, void *) {
    k_thread_name_set(k_current_get(), "Sensor_Task");
    logger.info("Sensor monitoring thread started");
    
    while (true) {
        sensorValue += (sys_rand32_get() % 10) + 1;
        
        sensorLogger.debug("Reading sensor... Value: %d", sensorValue);
        
        if (sensorValue > 50) {
            sensorLogger.warn("Sensor value high: %d - checking thresholds", sensorValue);
        }
        
        if (sensorValue > 80) {
            sensorLogger.error("CRITICAL! Sensor value too high: %d", sensorValue);
            sensorValue = 0; // Reset for demo
        }
        
        k_msleep(800);
    }
}

// Thread 2: Network Activity Logger
void network_thread_entry(void *, void *, void *) {
    k_thread_name_set(k_current_get(), "Network_Task");
    logger.info("Network monitoring thread started");
    
    while (true) {
        networkPackets += (sys_rand32_get() % 5) + 1;
        
        networkLogger.debug("Processing network packet #%d", networkPackets);
        
        if (networkPackets % 7 == 0) {
            networkLogger.warn("Network latency detected on packet %d", networkPackets);
        }
        
        if (networkPackets % 15 == 0) {
            networkLogger.error("Network timeout on packet %d - retrying", networkPackets);
        }
        
        k_msleep(600);
    }
}


// Define threads
K_THREAD_DEFINE(sensor_thread, 1024, sensor_thread_entry, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(network_thread, 1024, network_thread_entry, NULL, NULL, NULL, 7, 0, 0);

int main(void) {
    k_thread_name_set(k_current_get(), "Main_Task");
    logger.debug("NRF Zephyr Thread-Safe Logging Example Starting...");
    logger.info("ChronoLog Thread Safety: %s", CHRONOLOG_THREAD_SAFE ? "ENABLED" : "DISABLED");

    
    logger.warn("Multiple threads are now logging concurrently...");
    logger.info("Watch for interleaved log messages from different threads!");

    int mainLoopCounter = 0;
    while (true) {
        mainLoopCounter++;
        systemEvents++;
        
        logger.info("Main Loop: Iteration %d - System event #%d", mainLoopCounter, systemEvents);
        
        if (mainLoopCounter % 5 == 0) {
            logger.warn("Periodic system health check - Event %d", systemEvents);
        }
        
        k_msleep(3000);
    }

    return 0;
}
