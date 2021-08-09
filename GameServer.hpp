#pragma once
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
#define DEFAULT_PORT "27015"

/**Creates a multithreading-only server which waits for a client to connect with him and send him some messages.
**/

class GameServer {
protected:
    int iResult;
    int iSendResult;
    char* recvbuf;
    int recvbuflen;
    SOCKET ClientSocket;
    SOCKET ListenSocket;
    std::string* lastMessage;
    bool connected = false;
    bool wait = false;
    bool gotNewMessage = false;
    std::mutex* mutex;
public:

    GameServer() {
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

    void waitForClient() {
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

        this->receive();
    }



    void sendToClient(const char* message) {
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

    void receive() {
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

    inline std::string* getLastMessage() {
        return lastMessage;
    }

    inline bool isConnected() {
        return connected;
    }

    inline void bindMutex(std::mutex* i_mutex) {
        this->mutex = i_mutex;
    }

    inline std::mutex* getMutex() {
        return this->mutex;
    }


    inline bool newMessage() {
        bool temp = gotNewMessage;
        gotNewMessage = false;
        return temp;
    }
};