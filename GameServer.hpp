#pragma once

#include "Server.hpp"

class GameServer: public Server {
public:
    std::string* getLastMessage() {
        return lastMessage;
    }

    void sendToClient(const char* message) {
        iResult = send(ClientSocket, message, (int)strlen(message), 0);
        if (iResult == SOCKET_ERROR) {
            WSACleanup();
            return;
        }
        std::cout << "Server Message Sent: \n" << message;
    }

    void receive() {
        // Receive until the peer shuts down the connection
        while (true) {
            iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);

            //save message
            for (int i = 0; i < iResult; i++) {
                lastMessage = new std::string();
                lastMessage->push_back(recvbuf[i]);
            }

            std::cout << "Server received message: " << *lastMessage;
            if (iResult < 0) {
                std::cout << "Server recv failed with error: \n" << WSAGetLastError();
                closesocket(ClientSocket);
                WSACleanup();
                return;
            }
        }

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
};