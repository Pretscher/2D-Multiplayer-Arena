#pragma once
// Client side C/C++ program to demonstrate Socket programming
#include "iostream" 
using namespace std;
#include <vector>
#include <string>
#include <mutex>
#include <memory>
class PortableClient {
public:
    PortableClient();
    void receiveMultithreaded();
    void sendToServer(const char* message);

    shared_ptr<string> getLastMessage() const;
    bool isConnected() const;
    shared_ptr<mutex> getMutex() const;
    bool newMessage() const;

    string getIP() const;

    vector<string> avHosts;

    inline vector<string> getAvailableHosts() {
        vector<string> copy = avHosts;
        return std::move(copy);
    }

    void connectToHost(string ip);

    void searchHosts();
};