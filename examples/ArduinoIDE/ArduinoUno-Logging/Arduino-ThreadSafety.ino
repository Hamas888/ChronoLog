#include <Arduino.h>

#if defined(ESP32)
  #include <freertos/FreeRTOS.h>
  #include <freertos/task.h>
#endif

#define CHRONOLOG_THREAD_SAFE 1
#include "ChronoLog.h"

ChronoLogger logger("ThreadSafeLogger", CHRONOLOG_LEVEL_DEBUG);

#if defined(ESP32)
// Task handles
TaskHandle_t sensorTaskHandle       = NULL;
TaskHandle_t networkTaskHandle      = NULL;
TaskHandle_t systemTaskHandle       = NULL;

// Simulated sensor data
volatile int sensorValue            = 0;
volatile int networkPackets         = 0;
volatile int systemEvents           = 0;

// Task 1: Sensor Data Logger
void sensorTask(void *pvParameters) {
  int taskId = 1;
  logger.info("Task %d: Sensor monitoring task started on core %d", taskId, xPortGetCoreID());
  
  while (true) {
    sensorValue += random(1, 10);
    
    // Multiple log levels to show thread safety across different message types
    logger.debug("Task %d: Reading sensor... Value: %d", taskId, sensorValue);
    
    if (sensorValue > 50) {
      logger.warn("Task %d: Sensor value high: %d - checking thresholds", taskId, sensorValue);
    }
    
    if (sensorValue > 80) {
      logger.error("Task %d: CRITICAL! Sensor value too high: %d", taskId, sensorValue);
      sensorValue = 0; // Reset for demo
    }
    
    logger.info("Task %d: Sensor data logged successfully", taskId);
    vTaskDelay(pdMS_TO_TICKS(800)); // 800ms delay
  }
}

// Task 2: Network Activity Logger
void networkTask(void *pvParameters) {
  int taskId = 2;
  logger.info("Task %d: Network monitoring task started on core %d", taskId, xPortGetCoreID());
  
  while (true) {
    networkPackets += random(1, 5);
    
    logger.debug("Task %d: Processing network packet #%d", taskId, networkPackets);
    
    // Simulate different network events
    if (networkPackets % 7 == 0) {
      logger.warn("Task %d: Network latency detected on packet %d", taskId, networkPackets);
    }
    
    if (networkPackets % 15 == 0) {
      logger.error("Task %d: Network timeout on packet %d - retrying", taskId, networkPackets);
    }
    
    logger.info("Task %d: Network packet %d processed successfully", taskId, networkPackets);
    vTaskDelay(pdMS_TO_TICKS(600)); // 600ms delay
  }
}

// Task 3: System Events Logger
void systemTask(void *pvParameters) {
  int taskId = 3;
  logger.info("Task %d: System monitoring task started on core %d", taskId, xPortGetCoreID());
  
  while (true) {
    systemEvents++;
    
    logger.debug("Task %d: System event #%d triggered", taskId, systemEvents);
    
    // Simulate system events
    if (systemEvents % 5 == 0) {
      logger.info("Task %d: Memory usage check - Event %d", taskId, systemEvents);
    }
    
    if (systemEvents % 10 == 0) {
      logger.warn("Task %d: Periodic system health check - Event %d", taskId, systemEvents);
    }
    
    if (systemEvents % 20 == 0) {
      logger.error("Task %d: System maintenance required - Event %d", taskId, systemEvents);
    }
    
    vTaskDelay(pdMS_TO_TICKS(1200)); // 1200ms delay
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
  }

  /* 
    🕒 NOTE: If you sync system time to local time or UTC using NTP or another method,
    ChronoLog will automatically show real timestamps instead of MCU uptime-based logs 
  */

  logger.debug("Arduino Thread-Safe Logging Example...");
  logger.info("ChronoLog Thread Safety: %s", CHRONOLOG_THREAD_SAFE ? "ENABLED" : "DISABLED");
  logger.info("Available cores: %d", portNUM_PROCESSORS);
  logger.info("Logger initialized successfully at DEBUG level");
  
  logger.warn("Creating multiple tasks that will log concurrently...");
  logger.info("Watch for interleaved log messages from different tasks!");
  
  // Create multiple tasks on different cores to demonstrate thread safety
  xTaskCreatePinnedToCore(
    sensorTask,           // Task function
    "SensorTask",         // Task name
    4096,                 // Stack size
    NULL,                 // Task parameters
    2,                    // Priority
    &sensorTaskHandle,    // Task handle
    0                     // Core 0
  );
  
  xTaskCreatePinnedToCore(
    networkTask,          // Task function
    "NetworkTask",        // Task name
    4096,                 // Stack size
    NULL,                 // Task parameters
    2,                    // Priority
    &networkTaskHandle,   // Task handle
    1                     // Core 1
  );
  
  xTaskCreatePinnedToCore(
    systemTask,           // Task function
    "SystemTask",         // Task name
    4096,                 // Stack size
    NULL,                 // Task parameters
    1,                    // Priority (lower priority)
    &systemTaskHandle,    // Task handle
    0                     // Core 0 (same as sensor task)
  );
  
  logger.info("All tasks created successfully!");
  logger.info("Tasks running on different cores with different priorities");
  logger.debug("Task priorities: Sensor=2, Network=2, System=1");
}

void loop() {
  // Main loop logs periodically to show it's also thread-safe
  static int mainLoopCounter = 0;
  mainLoopCounter++;
  
  logger.info("Main Loop: Iteration %d - All tasks running smoothly", mainLoopCounter);
  
  // Show task states
  if (mainLoopCounter % 5 == 0) {
    logger.debug("Main Loop: Task states - Sensor: %s, Network: %s, System: %s",
                eTaskGetState(sensorTaskHandle) == eRunning ? "Running" : "Other",
                eTaskGetState(networkTaskHandle) == eRunning ? "Running" : "Other", 
                eTaskGetState(systemTaskHandle) == eRunning ? "Running" : "Other");
  }
  
  // Show memory info periodically
  if (mainLoopCounter % 10 == 0) {
    logger.warn("Main Loop: Free heap: %d bytes, Min free heap: %d bytes", 
                   ESP.getFreeHeap(), ESP.getMinFreeHeap());
  }
  
  delay(3000); // 3 second delay for main loop
}

#endif