/*
  ChronoLogRemote.cpp
  Implementation of the remote TCP log streaming server.
*/

#include "ChronoLog.h"

#if CHRONOLOG_REMOTE_ENABLE

ChronoLogRemote::ChronoLogRemote() : serverSocket(-1), running(false), port(0) {}

bool ChronoLogRemote::start(uint16_t port) {
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

void ChronoLogRemote::stop() {
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

void ChronoLogRemote::write(const char* data) {
    if (!running) return;

    #if CHRONOLOG_THREAD_SAFE
        std::lock_guard<std::mutex> lock(clientsMutex);
    #endif

    for (auto it = clients.begin(); it != clients.end();) {
        if (send(*it, data, strlen(data), 0) < 0) {
            close(*it);
            it = clients.erase(it);
        } else {
            ++it;
        }
    }
}

ChronoLogRemote::~ChronoLogRemote() {
    stop();
}

#endif // CHRONOLOG_REMOTE_ENABLE
