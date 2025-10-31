#define CHRONOLOG_REMOTE_ENABLE 1
#include "../../../include/ChronoLog.h"
#include "../../../include/ChronoLogRemote.h"
#include <thread>
#include <chrono>

int main() {
    // Start remote logging server on port 9999
    ChronoLogRemote::getInstance()->start(9999);
    printf("Remote logging server started on port 9999\n");
    printf("Connect using: nc localhost 9999\n");
    
    // Create some loggers
    ChronoLogger sensorLogger("Sensors", CHRONOLOG_LEVEL_DEBUG);
    ChronoLogger networkLogger("Network", CHRONOLOG_LEVEL_DEBUG);
    
    // Simulate some logging activity
    while(1) {
        sensorLogger.debug("Temperature reading: 25.6°C");
        networkLogger.info("Network status: Connected");
        networkLogger.warn("High latency detected: 150ms");
        
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
    return 0;
}