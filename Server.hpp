#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

/**Creates a multithreading-only server which waits for a client to connect with him and send him some messages.
**/
class Server {
public:
    Server() {
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
            return;
        }

        // Create a SOCKET for connecting to server
        ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (ListenSocket == INVALID_SOCKET) {
            std::cout << "Server socket failed with error: %ld\n" << WSAGetLastError();
            freeaddrinfo(result);
            WSACleanup();
            return;
        }

        // Setup the TCP listening socket
        iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            std::cout << "Server bind failed with error: \n" << WSAGetLastError();
            freeaddrinfo(result);
            closesocket(ListenSocket);
            WSACleanup();
            return;
        }

        freeaddrinfo(result);

        iResult = listen(ListenSocket, SOMAXCONN);
        if (iResult == SOCKET_ERROR) {
            std::cout << "Server listen failed with error: \n" << WSAGetLastError();
            closesocket(ListenSocket);
            WSACleanup();
            return;
        }

        std::cout << "Server successfully connected, waiting for client.";

        waitForClient();
    }

    void waitForClient() {
        // Accept a client socket
        ClientSocket = accept(ListenSocket, nullptr, nullptr);
        if (ClientSocket == INVALID_SOCKET) {
            std::cout << "Server accept failed with error: \n" << WSAGetLastError();
            closesocket(ListenSocket);
            WSACleanup();
            return;
        }

        // No longer need server socket
        closesocket(ListenSocket);

        std::cout << "Server successfully connected to client. Ready to receive messages";

        this->receive();
    }

    void receive() {
        // Receive until the peer shuts down the connection
        do {
            std::cout << "Server receiving. \n";
            iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
            if (iResult > 0) {
                std::cout << "Server Bytes received: \n" << iResult;

                // Echo the buffer back to the sender
                iSendResult = send(ClientSocket, recvbuf, iResult, 0);
                if (iSendResult == SOCKET_ERROR) {
                    std::cout << "Server send failed with error: \n" << WSAGetLastError();
                    closesocket(ClientSocket);
                    WSACleanup();
                    return;
                }
                std::cout << "Server Bytes sent: \n" << iSendResult;
            }
            else if (iResult == 0)
                std::cout << "Server Connection closing...\n";
            else {
                std::cout << "Server recv failed with error: \n" << WSAGetLastError();
                closesocket(ClientSocket);
                WSACleanup();
                return;
            }

        } while (iResult > 0);

        // shutdown the connection since we're done
        iResult = shutdown(ClientSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            std::cout << "Server shutdown failed with error: \n" << WSAGetLastError();
            closesocket(ClientSocket);
            WSACleanup();
            return;
        }

        // cleanup
        closesocket(ClientSocket);
        WSACleanup();
    }
private:
    int iResult;
    int iSendResult;
    char* recvbuf;
    int recvbuflen;
    SOCKET ClientSocket;
    SOCKET ListenSocket;
};
