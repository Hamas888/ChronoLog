#if defined(ESP32)
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define CHRONOLOG_REMOTE_ENABLE         1           // Enable remote logging server
#define CHRONOLOG_THREAD_SAFE           1           // Enabled thread-safe logging
#define CHRONOLOG_COLOR_ENABLE          1           // 0 = Disable colors for ANSII-incompatible terminals

#include "ChronoLog.h"

// WiFi credentials
#define WIFI_SSID      "YourSSID"
#define WIFI_PASS      "YourPassword"

ChronoLogger logger("RemoteLogger", CHRONOLOG_LEVEL_DEBUG);
ChronoLogger sensorLogger("Sensors", CHRONOLOG_LEVEL_DEBUG);
ChronoLogger networkLogger("Network", CHRONOLOG_LEVEL_DEBUG);

// Task handles
TaskHandle_t sensorTaskHandle       = NULL;
TaskHandle_t networkTaskHandle      = NULL;

// Task 1: Sensor Data Logger (Simulated)
void sensorTask(void *pvParameters) {
    logger.info("Sensor task started on core %d", xPortGetCoreID());
    
    while (true) {
        float temp = 20.0 + (rand() % 100) / 10.0;
        sensorLogger.debug("Temperature reading: %.1f°C", temp);
        
        if (temp > 28.0) {
            sensorLogger.warn("High temperature detected: %.1f°C", temp);
        }
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// Task 2: Network Activity Logger (Simulated)
void networkTask(void *pvParameters) {
    logger.info("Network task started on core %d", xPortGetCoreID());
    
    while (true) {
        networkLogger.info("Network status: Connected");
        
        if (rand() % 10 == 0) {
            networkLogger.error("Packet loss detected!");
        }
        
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }

    logger.info("Starting Arduino Remote Logging Example...");
    logger.info("ChronoLog Thread Safety: %s", CHRONOLOG_THREAD_SAFE ? "ENABLED" : "DISABLED");

    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    logger.info("Connecting to WiFi: %s", WIFI_SSID);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    logger.info("\nWiFi connected!");
    logger.info("IP Address: %s", WiFi.localIP().toString().c_str());

    // Start remote logging server on port 9999
    if (ChronoLogRemote::getInstance()->start(9999)) {
        logger.info("Remote logging server started on port 9999");
        logger.info("Connect using: nc %s 9999", WiFi.localIP().toString().c_str());
    } else {
        logger.error("Failed to start remote logging server!");
    }

    // Create tasks to demonstrate thread safety with remote logging
    // This mimics the "feel" of the ThreadSafety example
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

    logger.info("Tasks created and running concurrently on different cores.");
}

void loop() {
    // Main loop can also log, showing thread safety across tasks and main loop
    static int counter = 0;
    logger.info("Main loop heartbeat #%d", ++counter);
    
    // Show memory info periodically
    if (counter % 5 == 0) {
        logger.warn("Free heap: %d bytes", ESP.getFreeHeap());
    }
    
    delay(5000);
}

#else
void setup() {
    Serial.begin(115200);
    while (!Serial) {}
    Serial.println("This example requires an ESP32 for Remote Logging.");
}

void loop() {
    delay(1000);
}
#endif
