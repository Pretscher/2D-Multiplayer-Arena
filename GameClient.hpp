#pragma once
#include "Client.hpp"

class GameClient: public Client {
    using Client::Client;
public:

    void sendToServer(const char* message) {
        // Send an initial buffer
        iResult = send(ConnectSocket, message, (int)strlen(message), 0);
        if (iResult == SOCKET_ERROR) {
            std::cout << "Client send failed with error: \n" << WSAGetLastError();
            //closesocket(ConnectSocket);
            WSACleanup();
            return;
        }
        std::cout << "Client Message Sent: \n" << message;
    }

    void receive() {
        while (true) {
            iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
            //save message
            for (int i = 0; i < iResult; i++) {
                lastMessage = new std::string();
                lastMessage->push_back(recvbuf[i]);
            }
            std::cout << "Client received message: " << *lastMessage;
            if (iResult < 0) {
                std::cout << "Server recv failed with error: \n" << WSAGetLastError();
                closesocket(ConnectSocket);
                WSACleanup();
                return;
            }

        }

        // shutdown the connection since we're done
        iResult = shutdown(ConnectSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            std::cout << "Server shutdown failed with error: \n" << WSAGetLastError();
            closesocket(ConnectSocket);
            WSACleanup();
            return;
        }

        // cleanup
        closesocket(ConnectSocket);
        WSACleanup();
    }
};