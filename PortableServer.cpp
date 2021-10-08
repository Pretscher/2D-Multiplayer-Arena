#include "PortableServer.hpp"
#include <thread>
#include "iostream"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
using namespace std;

#ifdef __linux__ 

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <vector>
#include <mutex>
#include <string.h>
#define PORT 8080

static bool connected = false;
static int clientSocket;
static int server_fd;
static struct sockaddr_in address;
static int addrlen;
static int recvbuflen = 512;

static bool gotNewMessage = false;
static shared_ptr<string> lastMessage(new string());
static shared_ptr<mutex> mtx(new mutex());
static bool waitHandShaking = false;

PortableServer::PortableServer() {

    int opt = 1;
    addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        cout << "socket failed";
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
        &opt, sizeof(opt)))
    {
        cout << "setsockopt";
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address,
        sizeof(address)) < 0)
    {
        cout << "bind failed";
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        cout << "listen";
        exit(EXIT_FAILURE);
    }
}

void PortableServer::waitForClient() {
    if ((clientSocket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        cout << "accept";
        exit(EXIT_FAILURE);
    }
    connected = true;
}

void PortableServer::receiveMultithreaded() {
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(1));
        //received something
        char* recvbuf = new char[recvbuflen];
        int inputLenght = read(clientSocket, recvbuf, recvbuflen);
        //received a valid message
        if (inputLenght > 0) {
            mtx->lock();
            lastMessage->clear();
            gotNewMessage = true;
            //save message
            for (int i = 0; i < inputLenght; i++) {
                lastMessage->push_back(recvbuf[i]);
            }
            delete[] recvbuf;
            //connection setup
            if (lastMessage->compare("12345") == 0) {
                sendToClient("12345");
                lastMessage->clear();
                gotNewMessage = false;
            }
            mtx->unlock();
            waitHandShaking = false;
        }

        if (inputLenght < 0) {
            cout << "Lost connection to client.";
            setConnected(false);
            return;
        }
    }
}

void PortableServer::sendToClient(const char* message) {
    if (waitHandShaking == false) {
        send(clientSocket, message, strlen(message), 0);
        waitHandShaking = true;
    }
}

shared_ptr<string> PortableServer::getLastMessage() const {
    return lastMessage;
}

bool PortableServer::isConnected() const {
    return connected;
}

shared_ptr<mutex> PortableServer::getMutex() const {
    return mtx;
}


bool PortableServer::newMessage() const {
    bool temp = gotNewMessage;
    gotNewMessage = false;
    return temp;
}




#elif _WIN32
#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include <vector>
#include <mutex>
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

static string port = "8080";
static int recvbuflen = 512;
static SOCKET ClientSocket;
static SOCKET ListenSocket;
static shared_ptr<string> lastMessage;

static bool connected;

static mutex connectedMtx;
void setConnected(bool c) {
    connectedMtx.lock();
    connected = c;
    connectedMtx.unlock();
}
bool getConnected() {
    connectedMtx.lock();
    bool temp = connected;
    connectedMtx.unlock();
    return connected;
}

static bool wait;
static bool gotNewMessage;
static shared_ptr<mutex> mtx = shared_ptr<mutex>(new mutex());


PortableServer::PortableServer() {
    wait = false;
    gotNewMessage = false;
    connected = false;
    lastMessage = shared_ptr<string>(new string());

    WSADATA wsaData;

    ListenSocket = INVALID_SOCKET;
    ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cout << "Server WSAStartup failed with error: \n" << iResult;
        return;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, port.c_str(), &hints, &result);
    if (iResult != 0) {
        cout << "Server getaddrinfo failed with error: \n" << iResult;
        WSACleanup();
        exit(0);
        return;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "Server socket failed with error: %ld\n" << WSAGetLastError();
        freeaddrinfo(result);
        WSACleanup();
        exit(0);
        return;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        cout << "Server bind failed with error: \n" << WSAGetLastError();
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        exit(0);
        return;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        cout << "Server listen failed with error: \n" << WSAGetLastError();
        closesocket(ListenSocket);
        WSACleanup();
        exit(0);
        return;
    }

    cout << "Server successfully set up.\n";
}

void PortableServer::waitForClient() {
    // Accept a client socket

    ClientSocket = accept(ListenSocket, nullptr, nullptr);
    if (ClientSocket == INVALID_SOCKET) {
        cout << "Server accept failed with error: \n" << WSAGetLastError();
        closesocket(ListenSocket);
        WSACleanup();
        exit(0);
        return;
    }

    setConnected(true);

    // No long longer need server socket
    closesocket(ListenSocket);
    cout << "Server successfully connected to client. Ready to receive messages.\n";
}



void PortableServer::sendToClient(const char* message) {
    if (wait == false) {
        int iResult = send(ClientSocket, message, (int) strlen(message), 0);
        if (iResult == SOCKET_ERROR) {
            cout << "Server Message Sending Error: \n" << message;
            return;
        }
        wait = true;
    }
}

void PortableServer::receiveMultithreaded() {
    // Receive until the peer shuts down the connection
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(1));
        char* recvBuffer = new char[recvbuflen];
        int iResult = recv(ClientSocket, recvBuffer, recvbuflen, 0);

        //save message
        if (iResult > 0) {
            mtx->lock();//lock caus writing and reading message at the same time is not thread safe
            lastMessage->clear();
            gotNewMessage = true;
            //save message
            for (int i = 0; i < iResult; i++) {
                lastMessage->push_back(recvBuffer[i]);
            }
            delete[] recvBuffer;
            //connection setup
            if (lastMessage->compare("12345") == 0) {
                sendToClient("12345");
                lastMessage->clear();
                gotNewMessage = false;
            }

            mtx->unlock();
        }

        if (iResult < 0) {
            cout << "Lost connection to client.";
            closesocket(ClientSocket);
            setConnected(false);
            return;
        }
        wait = false;
    }
}

shared_ptr<string> PortableServer::getLastMessage() const {
    return lastMessage;
}

bool PortableServer::isConnected() const {
    return getConnected();
}

shared_ptr<mutex> PortableServer::getMutex() const {
    return mtx;
}


bool PortableServer::newMessage() const {
    bool temp = gotNewMessage;
    gotNewMessage = false;
    return temp;
}


string PortableServer::getIP() const {
    char hostname[255];
    struct hostent* he;
    struct in_addr** addr_list;

    WSAData data;
    WSAStartup(MAKEWORD(2, 2), &data);

    gethostname(hostname, 255);
    std::cout << "Host name: " << hostname << std::endl;

    if ((he = gethostbyname(hostname)) == NULL) {
        std::cout << "gethostbyname error" << std::endl;
        return string();
    }
    else {
        addr_list = (struct in_addr**) he->h_addr_list;
        return string(inet_ntoa(*addr_list[0]));
    }

}



#endif