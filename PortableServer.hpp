#pragma once
#include "iostream" 
using namespace std;
#include <string>
#include <mutex>
#include <memory>
#ifdef __linux__ 

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <ifaddrs.h>
#include <netdb.h>
#define PORT 8080


#elif _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#endif


class PortableServer {
public:
    PortableServer();
    void waitForClient();
  
    void receiveMultithreaded();
    void sendToClient(const char* message);

    shared_ptr<string> getLastMessage() const;
    shared_ptr<mutex> getMutex() const;
    bool newMessage();

    string getIP() const;

    bool isConnected() {
        connectedMtx.lock();
        bool temp = connected;
        connectedMtx.unlock();
        return temp;
    }
private:

#ifdef  __linux__ 
    int addrlen;
    int linClientSocket;
    struct sockaddr_in address;
#elif _WIN32
    SOCKET winClientSocket;
#endif

    string port = "8080";
    int recvbuflen = 512;

    shared_ptr<string> lastMessage;

    bool connected = false;

    mutex connectedMtx;
    bool wait;
    bool gotNewMessage;
    shared_ptr<mutex> mtx = shared_ptr<mutex>(new mutex());

    void setConnected(bool c) {
        connectedMtx.lock();
        connected = c;
        connectedMtx.unlock();
    }



    int portableSend(const char* message) const;
    int portableRecv(char* recvBuffer);
    void portableConnect();
    void portableShutdown();
};