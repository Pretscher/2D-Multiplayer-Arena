#pragma once
// Client side C/C++ program to demonstrate Socket programming
#include "iostream" 
using namespace std;
#include <string>
#include <mutex>

class PortableClient {
public:
    PortableClient(const char* serverIP);
    void waitForServer();
    void receiveMultithreaded();
    void sendToServer(const char* message);

    shared_ptr<string> getLastMessage() const;
    bool isConnected() const;
    shared_ptr<mutex> getMutex() const;
    bool newMessage() const;
};