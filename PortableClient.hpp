#pragma once
// Client side C/C++ program to demonstrate Socket programming

#include <string>
#include <mutex>

class PortableClient {
public:
    PortableClient(const char* serverIP);
    void waitForServer();
    void receiveMultithreaded();
    void sendToServer(const char* message);
    std::string* getLastMessage();
    bool isConnected();
    std::mutex* getMutex();
    bool newMessage();
};