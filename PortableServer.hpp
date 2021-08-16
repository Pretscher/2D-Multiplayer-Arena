#pragma once
#include <string>
#include <mutex>

class PortableServer {
public:
    PortableServer();
    void waitForClient();
  
    void receiveMultithreaded();
    void sendToClient(const char* message);

    std::string* getLastMessage();
    bool isConnected();
    std::mutex* getMutex();
    bool newMessage();
};