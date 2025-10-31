#include <WiFi.h>
#include <Arduino.h>

#define CHRONOLOG_REMOTE_ENABLE 1
#include "ChronoLog.h"

const char* ssid = "GhostPC";
const char* password = "Ghost899";

ChronoLogger logger("myLogger", CHRONOLOG_LEVEL_DEBUG);

void setup() {
    Serial.begin(115200);
    while (!Serial) {}                                                          // wait for serial port to connect. Needed for native USB}

    /* 
        🕒 NOTE: If you sync system time to local time or UTC using NTP or another method,
        ChronoLog will automatically show real timestamps instead of MCU uptime-based logs 
    */

    logger.debug("ESP32-S3 Remote Logging Example Starting...");
    logger.info("Logger initialized successfully at DEBUG level");

    logger.info("Connecting to WiFi SSID: %s", ssid);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        logger.debug("Trying to connect to WiFi...");
    }

    logger.info("WiFi connected");
    logger.info("IP address: %s", WiFi.localIP().toString().c_str());

    logger.info("Starting ChronoLog Remote Server on port 9999");
    if (ChronoLogRemote::getInstance()->start(9999)) {
        logger.info("ChronoLog Remote Server started successfully");
    } else {
        logger.error("Failed to start ChronoLog Remote Server");
    }

    logger.info("Run 'ncat %s 9999' on your PC to connect", WiFi.localIP().toString().c_str());
}

void loop() {
    logger.debug("This is a debug message");
    logger.info("This is an info message");
    logger.warn("This is a warning message");
    logger.error("This is an error message");
    logger.fatal("This is a fatal message");

    delay(5000);                                                                // Log messages every 5 seconds
}