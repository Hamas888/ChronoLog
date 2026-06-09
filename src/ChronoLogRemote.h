#ifndef CHRONOLOG_REMOTE_H
#define CHRONOLOG_REMOTE_H

#ifndef CHRONOLOG_REMOTE_ENABLE
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
    
    ChronoLogRemote() : serverSocket(-1), running(false), port(0) {}
    
public:
    static ChronoLogRemote* getInstance() {
        static ChronoLogRemote s_instance;
        return &s_instance;
    }
    
    bool start(uint16_t port = 9999) {
        if (running) return true;
        this->port = port;
        
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) return false;
        
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);
        
        int opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            close(serverSocket);
            return false;
        }
        
        if (listen(serverSocket, 3) < 0) {
            close(serverSocket);
            return false;
        }
        
        running = true;
        
        // Start accept thread
        std::thread([this]() {
            while (running) {
                struct sockaddr_in clientAddr;
                socklen_t clientLen = sizeof(clientAddr);
                int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
                
                if (clientSocket >= 0) {
                    #if CHRONOLOG_THREAD_SAFE
                        std::lock_guard<std::mutex> lock(clientsMutex);
                    #endif
                    clients.push_back(clientSocket);
                }
            }
        }).detach();
        
        return true;
    }
    
    void stop() {
        if (!running) return;
        running = false;
        
        #if CHRONOLOG_THREAD_SAFE
            std::lock_guard<std::mutex> lock(clientsMutex);
        #endif
        
        for (int client : clients) {
            close(client);
        }
        clients.clear();
        
        if (serverSocket >= 0) {
            close(serverSocket);
            serverSocket = -1;
        }
    }
    
    void write(const char* data) {
        if (!running) return;
        
        #if CHRONOLOG_THREAD_SAFE
            std::lock_guard<std::mutex> lock(clientsMutex);
        #endif
        
        // Use a reverse iterator to safely remove dead clients while iterating
        for (auto it = clients.begin(); it != clients.end();) {
            if (send(*it, data, strlen(data), 0) < 0) {
                close(*it);
                it = clients.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    ~ChronoLogRemote() {
        stop();
    }
};

// Note: single-instance is provided by function-local static in getInstance()

#endif // CHRONOLOG_REMOTE_ENABLE
#endif // CHRONOLOG_REMOTE_H