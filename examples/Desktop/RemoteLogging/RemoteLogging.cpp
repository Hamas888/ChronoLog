#define CHRONOLOG_REMOTE_ENABLE         1           // Enable remote logging server
#define CHRONOLOG_COLOR_ENABLE          1           // 0 = Disable colors for ANSII-incompatible terminals

#include "../../../include/ChronoLog.h"
#include "../../../include/ChronoLogRemote.h"
#include <thread>
#include <chrono>

int main() {
    // Start remote logging server on port 9999
    ChronoLogRemote::getInstance()->start(9999);
    
    ChronoLogger logger("RemoteLogger", CHRONOLOG_LEVEL_DEBUG);
    logger.debug("Desktop Remote Logging Example Starting...");
    logger.info("Remote logging server started on port 9999");
    logger.info("Connect using: nc localhost 9999");

    // Create some loggers
    ChronoLogger sensorLogger("Sensors", CHRONOLOG_LEVEL_DEBUG);
    ChronoLogger networkLogger("Network", CHRONOLOG_LEVEL_DEBUG);
    
    // Simulate some logging activity
    while(true) {
        sensorLogger.debug("Temperature reading: 25.6°C");
        networkLogger.info("Network status: Connected");
        networkLogger.warn("High latency detected: 150ms");
        
        std::this_thread::sleep_for(std::chrono::seconds(2));                  // Log messages every 2 seconds
    }
    
    return 0;
}
