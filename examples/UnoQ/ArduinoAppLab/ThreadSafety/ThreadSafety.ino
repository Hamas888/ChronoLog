#include <zephyr/kernel.h>
#include <Arduino_RouterBridge.h>

#define CHRONOLOG_THREAD_SAFE           1           // Enabled thread-safe logging
#define CHRONOLOG_COLOR_ENABLE          1           // 0 = Disable colors for ANSII-incompatible terminals

#include "ChronoLog.h"

// Thread structures
k_tid_t             sensorThreadId          = NULL;
k_tid_t             networkThreadId         = NULL;
k_tid_t             systemThreadId          = NULL;

struct k_thread     sensorThread;
struct k_thread     networkThread;
struct k_thread     systemThread;

k_thread_stack_t    *sensorThreadStack;
k_thread_stack_t    *networkThreadStack;
k_thread_stack_t    *systemThreadStack;

// Simulated sensor data
volatile int        sensorValue             = 0;
volatile int        networkPackets          = 0;
volatile int        systemEvents            = 0;

// Create ChronoLogger global instance
ChronoLogger logger("ThreadSafeLogger", CHRONOLOG_LEVEL_DEBUG);

// Thread 1: Sensor Data Logger
void sensorTask(void* p1, void* p2, void* p3) {
  int taskId = 1;
  logger.setThreadName("Sensor_Task");
  logger.info("Task %d: Sensor monitoring task started", taskId);
  
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
    k_msleep(800); // 800ms delay
  }
}

// Thread 2: Network Activity Logger
void networkTask(void* p1, void* p2, void* p3) {
  int taskId = 2;
  logger.setThreadName("Network_Task");
  logger.info("Task %d: Network monitoring task started", taskId);
  
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
    k_msleep(600); // 600ms delay
  }
}

// Thread 3: System Events Logger
void systemTask(void* p1, void* p2, void* p3) {
  int taskId = 3;
  logger.setThreadName("System_Task");
  logger.info("Task %d: System monitoring task started", taskId);
  
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
    
    k_msleep(1200); // 1200ms delay
  }
}

void setup() {
  Monitor.begin();
  while (!Monitor) {
  }

  logger.setThreadName("Main_Loop");
  
  logger.debug("Arduino Uno Q Thread-Safe Logging Example...");
  logger.info("ChronoLog Thread Safety: %s", CHRONOLOG_THREAD_SAFE ? "ENABLED" : "DISABLED");
  logger.info("Logger initialized successfully at DEBUG level");
  
  logger.warn("Creating multiple tasks that will log concurrently...");
  logger.info("Watch for interleaved log messages from different tasks!");
  
  // Allocate thread stacks
  sensorThreadStack     = (k_thread_stack_t*)k_malloc(K_THREAD_STACK_LEN(2048));
  systemThreadStack     = (k_thread_stack_t*)k_malloc(K_THREAD_STACK_LEN(2048));
  networkThreadStack    = (k_thread_stack_t*)k_malloc(K_THREAD_STACK_LEN(2048));
  
  if (!sensorThreadStack || !networkThreadStack || !systemThreadStack) {
    logger.error("Failed to allocate thread stacks!");
    return;
  }
  
  // Create multiple threads with different priorities to demonstrate thread safety
  sensorThreadId = k_thread_create(
    &sensorThread,
    sensorThreadStack,
    2048,
    sensorTask,
    NULL, NULL, NULL,
    5,                    // Priority (lower number = higher priority)
    0,
    K_NO_WAIT
  );
  k_thread_name_set(sensorThreadId, "Sensor_Task");
  
  networkThreadId = k_thread_create(
    &networkThread,
    networkThreadStack,
    2048,
    networkTask,
    NULL, NULL, NULL,
    5,                    // Priority (same as sensor)
    0,
    K_NO_WAIT
  );
  k_thread_name_set(networkThreadId, "Network_Task");
  
  systemThreadId = k_thread_create(
    &systemThread,
    systemThreadStack,
    2048,
    systemTask,
    NULL, NULL, NULL,
    7,                    // Priority (lower priority)
    0,
    K_NO_WAIT
  );
  k_thread_name_set(systemThreadId, "System_Task");
  
  logger.info("All tasks created successfully!");
  logger.info("Tasks running with different priorities");
  logger.debug("Task priorities: Sensor=5, Network=5, System=7");
}

void loop() {
  // Main loop logs periodically to show it's also thread-safe
  static int mainLoopCounter = 0;
  mainLoopCounter++;
  
  logger.info("Main Loop: Iteration %d - All tasks running smoothly", mainLoopCounter);
  
  // Show task states periodically
  if (mainLoopCounter % 5 == 0) {
    uint8_t sensorState = k_thread_priority_get(sensorThreadId);
    uint8_t systemState = k_thread_priority_get(systemThreadId);
    uint8_t networkState = k_thread_priority_get(networkThreadId);
    
    logger.debug(
        "Main Loop: Task priorities - Sensor: %d, Network: %d, System: %d",
        sensorState, networkState, systemState
    );
  }
  
  // Show statistics periodically
  if (mainLoopCounter % 10 == 0) {
    logger.warn(
        "Main Loop: Data counters - Sensor: %d, Network: %d, System: %d", 
        sensorValue, networkPackets, systemEvents
    );
  }
  
  k_msleep(3000); // 3 second delay for main loop
}