#ifndef CHRONOLOG_REMOTE_H
#define CHRONOLOG_REMOTE_H

#ifndef CHRONOLOG_REMOTE_ENABLE
  #warning "CHRONOLOG_REMOTE_ENABLE is not defined. Defaulting to 0 (remote logging disabled)."
  #define CHRONOLOG_REMOTE_ENABLE  0    // Set to 1 to enable remote logging
#endif

#if CHRONOLOG_REMOTE_ENABLE

#if defined(CHRONOLOG_PLATFORM_DESKTOP)
  // Standard C++ headers
  #include <vector>
  #include <algorithm>
  #include <cstring>
  #include <thread>
  #include <mutex>
  // System headers
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #include <fcntl.h>
#elif defined(CHRONOLOG_ESP) || defined(CHRONOLOG_PLATFORM_ESP_IDF)
  // Standard C++ headers
  #include <vector>
  #include <algorithm>
  #include <cstring>
  #include <thread>
  #include <mutex>
  // ESP-IDF headers
  #include "lwip/sockets.h"
  #include "lwip/netdb.h"
  #include "esp_wifi.h"
  #include "esp_event.h"
#endif

class ChronoLogRemote {
private:
    int serverSocket;
    std::vector<int> clients;
    bool running;
    uint16_t port;

    #if CHRONOLOG_THREAD_SAFE
        std::mutex clientsMutex;
    #endif

    ChronoLogRemote();

public:
    static ChronoLogRemote* getInstance() {
        static ChronoLogRemote s_instance;
        return &s_instance;
    }

    bool start(uint16_t port = 9999);
    void stop();
    void write(const char* data);
    ~ChronoLogRemote();

    // Non-copyable, non-movable
    ChronoLogRemote(const ChronoLogRemote&) = delete;
    ChronoLogRemote& operator=(const ChronoLogRemote&) = delete;
};

#endif // CHRONOLOG_REMOTE_ENABLE
#endif // CHRONOLOG_REMOTE_H