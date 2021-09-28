#pragma once
// Client side C/C++ program to demonstrate Socket programming
#include "iostream" 
using namespace std;
#include <string>
#include <mutex>
#include <memory>
class PortableClient {
public:
    PortableClient();
    void waitForServer();
    void receiveMultithreaded();
    void sendToServer(const char* message);

    shared_ptr<string> getLastMessage() const;
    bool isConnected() const;
    shared_ptr<mutex> getMutex() const;
    bool newMessage() const;

    string getIP() const;
    string searchHost() const;
};