#include "PortableServer.hpp"


#ifdef __linux__ 

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <mutex>

#define PORT 8080

static bool connected = false;
static int clientSocket;
static int server_fd;
static struct sockaddr_in address;
static int addrlen;
static int inputLenght;
static int bufferMaxLenght = 512;
static char* inputBuffer = new char[bufferMaxLenght];
static bool gotNewMessage = false;
static std::string* lastMessage;
static std::mutex* writingMessage = new std::mutex();
static bool waitHandShaking = false;

PortableServer::PortableServer() {

    int opt = 1;
    addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        std::cout << "socket failed";
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
        &opt, sizeof(opt)))
    {
        std::cout << "setsockopt";
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address,
        sizeof(address)) < 0)
    {
        std::cout << "bind failed";
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        std::cout << "listen";
        exit(EXIT_FAILURE);
    }
}

void PortableServer::waitForClient() {
    if ((clientSocket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        std::cout << "accept";
        exit(EXIT_FAILURE);
    }
    connected = true;
}

void PortableServer::receiveMultithreaded() {
    while (true) {

        //received something
        inputLenght = read(clientSocket, inputBuffer, bufferMaxLenght);
        //received a valid message
        if (inputLenght > 0) {
            writingMessage->lock();
            if (lastMessage != nullptr) delete lastMessage;
            lastMessage = new std::string();
            gotNewMessage = true;
            //save message
            for (int i = 0; i < inputLenght; i++) {
                lastMessage->push_back(inputBuffer[i]);
            }
            writingMessage->unlock();
            waitHandShaking = false;
        }

        if (inputLenght < 0) {
            std::cout << "error, server received message with negative lenght";
            std::exit(0);
        }
    }
}

void PortableServer::sendToClient(const char* message) {
    if (waitHandShaking == false) {
        send(clientSocket, message, strlen(message), 0);
        waitHandShaking = true;
    }
}

std::string* PortableServer::getLastMessage() {
    return lastMessage;
}

bool PortableServer::isConnected() {
    return connected;
}

std::mutex* PortableServer::getMutex() {
    return writingMessage;
}

bool PortableServer::newMessage() {
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
#include <iostream>
#include <vector>
#include <mutex>
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "8080"

static int iResult;
static int iSendResult;
static char* recvbuf;
static int recvbuflen;
static SOCKET ClientSocket;
static SOCKET ListenSocket;
static std::string* lastMessage;
static bool connected = false;
static bool wait = false;
static bool gotNewMessage = false;
static std::mutex* mutex = new std::mutex();

PortableServer::PortableServer() {
    lastMessage = nullptr;
    WSADATA wsaData;

    ListenSocket = INVALID_SOCKET;
    ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;


    recvbuf = new char[DEFAULT_BUFLEN];
    recvbuflen = DEFAULT_BUFLEN;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cout << "Server WSAStartup failed with error: \n" << iResult;
        return;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        std::cout << "Server getaddrinfo failed with error: \n" << iResult;
        WSACleanup();
        std::exit(0);
        return;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        std::cout << "Server socket failed with error: %ld\n" << WSAGetLastError();
        freeaddrinfo(result);
        WSACleanup();
        std::exit(0);
        return;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        std::cout << "Server bind failed with error: \n" << WSAGetLastError();
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        std::exit(0);
        return;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        std::cout << "Server listen failed with error: \n" << WSAGetLastError();
        closesocket(ListenSocket);
        WSACleanup();
        std::exit(0);
        return;
    }

    std::cout << "Server successfully set up.\n";
}

void PortableServer::waitForClient() {
    // Accept a client socket
    ClientSocket = accept(ListenSocket, nullptr, nullptr);
    if (ClientSocket == INVALID_SOCKET) {
        std::cout << "Server accept failed with error: \n" << WSAGetLastError();
        closesocket(ListenSocket);
        WSACleanup();
        std::exit(0);
        return;
    }

    connected = true;
    // No longer need server socket
    closesocket(ListenSocket);
    std::cout << "Server successfully connected to client. Ready to receive messages.\n";
}



void PortableServer::sendToClient(const char* message) {
    if (wait == false) {
        iResult = send(ClientSocket, message, (int)strlen(message), 0);
        if (iResult == SOCKET_ERROR) {
            std::cout << "Server Message Sending Error: \n" << message;
            WSACleanup();
            std::exit(0);
            return;
            }
        wait = true;
        //    std::cout << "Server Message Sent: \n" << std::string(message);
        }
    }

void PortableServer::receiveMultithreaded() {
    // Receive until the peer shuts down the connection
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);


        //save message
        if (iResult > 0) {
            mutex->lock();//lock caus writing and reading message at the same time is not thread safe
            if (lastMessage != nullptr) delete lastMessage;
            lastMessage = new std::string();
            gotNewMessage = true;
            //save message
            for (int i = 0; i < iResult; i++) {
                lastMessage->push_back(recvbuf[i]);
            }
            mutex->unlock();
        }

        if (iResult < 0) {
            std::cout << "Server recv failed with error: \n" << WSAGetLastError();
            closesocket(ClientSocket);
            WSACleanup();
            std::exit(0);
            return;
        }
        wait = false;
        //   std::cout << "Server received message: " << *lastMessage;
    }

    std::exit(0);
}

std::string* PortableServer::getLastMessage() {
    return lastMessage;
}

bool PortableServer::isConnected() {
    return connected;
}

std::mutex* PortableServer::getMutex() {
    return mutex;
}


bool PortableServer::newMessage() {
    bool temp = gotNewMessage;
    gotNewMessage = false;
    return temp;
}
#endif