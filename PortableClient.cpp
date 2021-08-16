#include "PortableClient.hpp"

#ifdef __linux__ 


#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <mutex>

#define PORT 8080

static bool connected = false;
static int serverSocket;
static int server_fd;
static struct sockaddr_in serv_addr;
static int addrlen;
static int inputLenght = 0;
static int bufferMaxLenght = 512;
static char* inputBuffer = new char[bufferMaxLenght];

static bool gotNewMessage = false;
static std::string* lastMessage;
static std::mutex* writingMessage = new std::mutex();
static bool waitHandShaking = false;

PortableClient::PortableClient(const char* serverIP) {

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        std::exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, serverIP, &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        std::exit(0);
    }
    }

void PortableClient::waitForServer() {
    if (connect(serverSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        std::exit(0);
    }
    connected = true;
}

void PortableClient::receiveMultithreaded() {
    while (true) {
        inputLenght = read(serverSocket, inputBuffer, bufferMaxLenght);
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
            std::cout << "error, client received message with negative lenght";
            std::exit(0);
        }
    }
}

void PortableClient::sendToServer(const char* message) {
    if (waitHandShaking == false) {
        send(serverSocket, message, strlen(message), 0);
        waitHandShaking = true;
    }
}

std::string* PortableClient::getLastMessage() {
    return lastMessage;
}

bool PortableClient::isConnected() {
    return connected;
}

std::mutex* PortableClient::getMutex() {
    return writingMessage;
}

bool PortableClient::newMessage() {
    bool temp = gotNewMessage;
    gotNewMessage = false;
    return temp;
}


#elif _WIN32

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <mutex>
// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

static int iResult;
static SOCKET ConnectSocket;
static char* recvbuf;
static int recvbuflen;
static std::string* lastMessage;
static bool connected = false;
static bool wait = false;
static std::mutex* mutex = new std::mutex();
static bool gotNewMessage = false;

PortableClient::PortableClient(const char* serverIP) {
    lastMessage = nullptr;
    WSADATA wsaData;
    ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
    recvbuf = new char[DEFAULT_BUFLEN];

    recvbuflen = DEFAULT_BUFLEN;


    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cout << "Client WSAStartup failed with error: \n" << iResult;
        return;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(serverIP, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        std::cout << "Client getaddrinfo failed with error: \n" << iResult;
        WSACleanup();
        return;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            std::cout << "Client socket connection failed with error: %ld\n" << WSAGetLastError();
            WSACleanup();
            std::exit(0);
            return;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

}

void PortableClient::waitForServer() {
    if (ConnectSocket == INVALID_SOCKET) {
        std::cout << "Client Unable to connect to server!\n";
        WSACleanup();
        std::exit(0);
        return;
    }
    connected = true;
}

bool PortableClient::isConnected() {
    return connected;
}

void PortableClient::sendToServer(const char* message) {
    if (wait == false) {
        // Send an initial buffer
        iResult = send(ConnectSocket, message, (int)strlen(message), 0);
        if (iResult == SOCKET_ERROR) {
            std::cout << "Client send failed with error: \n" << WSAGetLastError();
            WSACleanup();
            std::exit(0);
            return;
        }
        wait = true;
        //  std::cout << "Client Message Sent: \n" << message;
    }
}

void PortableClient::receiveMultithreaded() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);


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

        // std::cout << "Client received message: " << *lastMessage;
        if (iResult < 0) {
            std::cout << "Client recv failed with error: \n" << WSAGetLastError();
            closesocket(ConnectSocket);
            WSACleanup();
            std::exit(0);
            return;
        }
        wait = false;
    }

    // shutdown the connection since we're done
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        std::cout << "Client shutdown failed with error: \n" << WSAGetLastError();
        closesocket(ConnectSocket);
        WSACleanup();
        std::exit(0);
        return;
    }

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();
}

std::string* PortableClient::getLastMessage() {
    return lastMessage;
}

std::mutex* PortableClient::getMutex() {
    return mutex;
}

bool PortableClient::newMessage() {
    bool temp = gotNewMessage;
    gotNewMessage = false;
    return temp;
}

#endif