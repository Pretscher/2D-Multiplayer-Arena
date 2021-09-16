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
    string* getLastMessage();
    bool isConnected();
    mutex* getMutex();
    bool newMessage();
};