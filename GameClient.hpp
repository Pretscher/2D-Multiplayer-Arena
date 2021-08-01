#pragma once
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

class GameClient {
protected:
    int iResult;
    SOCKET ConnectSocket;
    char* recvbuf;

    int recvbuflen;
    std::string* lastMessage;
    bool connected = false;
    bool wait = false;
    std::mutex* mutex;

public:
    GameClient(const char* serverIP) {
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

        if (ConnectSocket == INVALID_SOCKET) {
            std::cout << "Client Unable to connect to server!\n";
            WSACleanup();
            std::exit(0);
            return;
        }

        connected = true;
    }

    bool isConnected() {
        return connected;
    }

    void sendToServer(const char* message) {
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

    void receive() {
        while (true) {
            iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);

            mutex->lock();//lock caus writing and reading message at the same time is not thread safe
            if (lastMessage != nullptr) delete lastMessage;
            lastMessage = new std::string();
            //save message
            for (int i = 0; i < iResult; i++) {
                lastMessage->push_back(recvbuf[i]);
            }
            mutex->unlock();

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

    std::string* getLastMessage() {
        return lastMessage;
    }

    void bindMutex(std::mutex* i_mutex) {
        this->mutex = i_mutex;
    }
};