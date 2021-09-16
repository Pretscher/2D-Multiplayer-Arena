#pragma once
#include "iostream" 
using namespace std;
#include <string>
#include <mutex>

class PortableServer {
public:
    PortableServer();
    void waitForClient();
  
    void receiveMultithreaded();
    void sendToClient(const char* message);

    string* getLastMessage();
    bool isConnected();
    mutex* getMutex();
    bool newMessage();
};