#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_random.h"

#define CHRONOLOG_THREAD_SAFE           1           // Enabled thread-safe logging
#define CHRONOLOG_COLOR_ENABLE          1           // 0 = Disable colors for ANSII-incompatible terminals
#include "ChronoLog.h"

static ChronoLogger                     logger("System", CHRONOLOG_LEVEL_DEBUG);
static ChronoLogger                     sensorLogger("Sensors", CHRONOLOG_LEVEL_DEBUG);
static ChronoLogger                     networkLogger("Network", CHRONOLOG_LEVEL_DEBUG);

// Task handles
TaskHandle_t                            sensorTaskHandle  = NULL;
TaskHandle_t                            networkTaskHandle = NULL;
TaskHandle_t                            systemTaskHandle  = NULL;

// Simulated sensor data
volatile int                            sensorValue       = 0;
volatile int                            networkPackets    = 0;
volatile int                            systemEvents      = 0;

// Task 1: Sensor Data Logger
void sensorTask(void *pvParameters) {
    sensorLogger.info("Sensor monitoring task started on core %d", xPortGetCoreID());
    
    while (true) {
        sensorValue += (esp_random() % 10) + 1;
        
        // Multiple log levels to show thread safety across different message types
        sensorLogger.debug("Reading sensor... Value: %d", sensorValue);
        
        if (sensorValue > 50) {
            sensorLogger.warn("Sensor value high: %d - checking thresholds", sensorValue);
        }
        
        if (sensorValue > 80) {
            sensorLogger.error("CRITICAL! Sensor value too high: %d", sensorValue);
            sensorValue = 0; // Reset for demo
        }
        
        sensorLogger.info("Sensor data logged successfully");
        vTaskDelay(pdMS_TO_TICKS(800)); // 800ms delay
    }
}

// Task 2: Network Activity Logger
void networkTask(void *pvParameters) {
    networkLogger.info("Network monitoring task started on core %d", xPortGetCoreID());
    
    while (true) {
        networkPackets += (esp_random() % 5) + 1;
        
        networkLogger.debug("Processing network packet #%d", networkPackets);
        
        // Simulate different network events
        if (networkPackets % 7 == 0) {
            networkLogger.warn("Network latency detected on packet %d", networkPackets);
        }
        
        if (networkPackets % 15 == 0) {
            networkLogger.error("Network timeout on packet %d - retrying", networkPackets);
        }
        
        networkLogger.info("Network packet %d processed successfully", networkPackets);
        vTaskDelay(pdMS_TO_TICKS(600)); // 600ms delay
    }
}

// Task 3: System Events Logger
void systemTask(void *pvParameters) {
    logger.info("System monitoring task started on core %d", xPortGetCoreID());
    
    while (true) {
        systemEvents++;
        
        logger.debug("System event #%d triggered", systemEvents);
        
        // Simulate system events
        if (systemEvents % 5 == 0) {
            logger.info("Memory usage check - Event %d", systemEvents);
        }
        
        if (systemEvents % 10 == 0) {
            logger.warn("Periodic system health check - Event %d", systemEvents);
        }
        
        if (systemEvents % 20 == 0) {
            logger.error("System maintenance required - Event %d", systemEvents);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1200)); // 1200ms delay
    }
}

extern "C" void app_main(void) {
    logger.debug("ESP-IDF Thread-Safe Logging Example...");
    logger.info("ChronoLog Thread Safety: %s", CHRONOLOG_THREAD_SAFE ? "ENABLED" : "DISABLED");
    
    logger.warn("Creating multiple tasks that will log concurrently...");
    logger.info("Watch for interleaved log messages from different tasks!");
    
    // Create multiple tasks on different cores to demonstrate thread safety
    xTaskCreatePinnedToCore(sensorTask,  "SensorTask",  4096, NULL, 2, &sensorTaskHandle,  0);
    xTaskCreatePinnedToCore(networkTask, "NetworkTask", 4096, NULL, 2, &networkTaskHandle, 1);
    xTaskCreatePinnedToCore(systemTask,  "SystemTask",  4096, NULL, 1, &systemTaskHandle,  0);
    
    logger.info("All tasks created successfully!");
    
    // Main loop logs periodically to show it's also thread-safe
    int mainLoopCounter = 0;
    while (true) {
        mainLoopCounter++;
        logger.info("Main Loop: Iteration %d - All tasks running smoothly", mainLoopCounter);
        
        // Show memory info periodically
        if (mainLoopCounter % 10 == 0) {
            logger.warn("Free heap: %d bytes", esp_get_free_heap_size());
        }
        
        vTaskDelay(pdMS_TO_TICKS(3000)); // 3 second delay
    }
}

