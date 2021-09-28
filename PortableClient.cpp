#include "PortableClient.hpp"
#include <iostream>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
using namespace std;

#ifdef __linux__ 


#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <mutex>
#include <thread>
#include <vector>

#define PORT 8080

static bool connected = false;
static int serverSocket;
static int server_fd;
static struct sockaddr_in serv_addr;
static int addrlen;
static int inputLenght = 0;
static int bufferMaxLenght = 512;
static vector<char> inputBuffer(bufferMaxLenght);

static bool gotNewMessage = false;
static shared_ptr<string> lastMessage(new string());
static shared_ptr<mutex> writingMessage(new mutex());
static bool waitHandShaking = false;

PortableClient::PortableClient(const char* serverIP) {

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, serverIP, &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        exit(0);
    }
    }

void PortableClient::waitForServer() {
    if (connect(serverSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        exit(0);
    }
    connected = true;
}

void PortableClient::receiveMultithreaded() {
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(1));
        inputLenght = read(serverSocket, inputBuffer.data(), bufferMaxLenght);
        //received a valid message
        if (inputLenght > 0) {
            writingMessage->lock();
            lastMessage->clear();
            gotNewMessage = true;
            //save message
            for (int i = 0; i < inputLenght; i++) {
                lastMessage->push_back(inputBuffer[i]);
            }
            writingMessage->unlock();
            waitHandShaking = false;
        }

        if (inputLenght < 0) {
            cout << "error, client received message with negative lenght";
            exit(0);
        }
    }
}

void PortableClient::sendToServer(const char* message) {
    if (waitHandShaking == false) {
        send(serverSocket, message, strlen(message), 0);
        waitHandShaking = true;
    }
}

bool PortableClient::isConnected() const {
    return connected;
}
shared_ptr<string> PortableClient::getLastMessage() const {
    return lastMessage;
}

shared_ptr<mutex> PortableClient::getMutex() const {
    return writingMessage;
}

bool PortableClient::newMessage() const {
    bool temp = gotNewMessage;
    gotNewMessage = false;
    return temp;
}


#elif _WIN32

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <mutex>
#include <vector>
// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "8080"

static int iResult;
static SOCKET ConnectSocket;
static vector<char> recvbuf;
static int recvbuflen;
static shared_ptr<string> lastMessage = shared_ptr<string>(new string());
static bool connected = false;
static bool wait = false;
static shared_ptr<mutex> mtx = shared_ptr<mutex>(new mutex());
static bool gotNewMessage = false;

PortableClient::PortableClient() {
    WSADATA wsaData;
    ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
    recvbuf = vector<char>(DEFAULT_BUFLEN);

    recvbuflen = DEFAULT_BUFLEN;


    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cout << "Client WSAStartup failed with error: \n" << iResult;
        return;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    string copy = searchHost();
    if (copy == "") {
        std::cout << "No host found";
        std::exit(0);
    }
    // Resolve the server address and port
    iResult = getaddrinfo(copy.c_str(), DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        cout << "Client getaddrinfo failed with error: \n" << iResult;
        WSACleanup();
        return;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            cout << "Client socket connection failed with error: %ld\n" << WSAGetLastError();
            WSACleanup();
            exit(0);
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
        cout << "Client Unable to connect to server!\n";
        WSACleanup();
        exit(0);
        return;
    }
    connected = true;
}


void PortableClient::sendToServer(const char* message) {
    if (wait == false) {
        // Send an initial buffer
        iResult = send(ConnectSocket, message, (int)strlen(message), 0);
        if (iResult == SOCKET_ERROR) {
            cout << "Client send failed with error: \n" << WSAGetLastError();
            WSACleanup();
            exit(0);
            return;
        }
        wait = true;
        //  cout << "Client Message Sent: \n" << message;
    }
}

void PortableClient::receiveMultithreaded() {
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(1));
        iResult = recv(ConnectSocket, recvbuf.data(), recvbuflen, 0);


        if (iResult > 0) {
            mtx->lock();//lock caus writing and reading message at the same time is not thread safe
            lastMessage->clear();
            gotNewMessage = true;

            //save message
            for (int i = 0; i < iResult; i++) {
                lastMessage->push_back(recvbuf[i]);
            }

            mtx->unlock();
        }

        // cout << "Client received message: " << *lastMessage;
        if (iResult < 0) {
            cout << "Client recv failed with error: \n" << WSAGetLastError();
            closesocket(ConnectSocket);
            WSACleanup();
            exit(0);
            return;
        }
        wait = false;
    }

    // shutdown the connection since we're done
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        cout << "Client shutdown failed with error: \n" << WSAGetLastError();
        closesocket(ConnectSocket);
        WSACleanup();
        exit(0);
        return;
    }

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();
}

shared_ptr<string> PortableClient::getLastMessage() const {
    return lastMessage;
}

shared_ptr<mutex> PortableClient::getMutex() const {
    return mtx;
}

bool PortableClient::isConnected() const {
    return connected;
}

bool PortableClient::newMessage() const {
    bool temp = gotNewMessage;
    gotNewMessage = false;
    return temp;
}

string PortableClient::getIP() const {
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



const unsigned int checkedIpCount = 256;
thread** threads;
mutex** mutices;
bool* threadFinished;
string foundIP = "";
void testIP(const char* myIP, struct addrinfo* result, struct addrinfo* hints, int index) {
    SOCKET ConnectSocket;
    int res;
    res = getaddrinfo(myIP, DEFAULT_PORT, hints, &result);
    if (result != nullptr) {
        if (iResult != 0) {
            cout << "Client getaddrinfo failed with error: \n" << res;
            WSACleanup();
        }

        // Attempt to connect to an address until one succeeds
        // Create a SOCKET for connecting to server

    
        ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            cout << "Client socket connection failed with error: %ld\n" << WSAGetLastError();
            WSACleanup();
        }

        res = connect(ConnectSocket, result->ai_addr, (int) result->ai_addrlen);
        if (res == SOCKET_ERROR) {
            closesocket(ConnectSocket);
        }
        else {
            iResult = send(ConnectSocket, "12345", (int) strlen("12345"), 0);
            if (iResult == SOCKET_ERROR) {
                cout << "Client send failed with error: \n" << WSAGetLastError();
                WSACleanup();
            }

            long long startTime = std::chrono::system_clock::now().time_since_epoch().count();
            while (std::chrono::system_clock::now().time_since_epoch().count() - startTime < 1000) {
                this_thread::sleep_for(chrono::milliseconds(1));
                iResult = recv(ConnectSocket, recvbuf.data(), recvbuflen, 0);

                if (iResult > 0) {
                    foundIP = myIP;
                    return;//dont set threadFinished to true so that no multithreading error can occur where the filled string is ignored
                }

                // cout << "Client received message: " << *lastMessage;
                if (iResult < 0) {
                    cout << "Client recv failed with error: \n" << WSAGetLastError();
                    closesocket(ConnectSocket);
                    WSACleanup();
                    exit(0);
                    return;
                }
            }
        }
    }
    mutices[index]->lock();
    threadFinished[index] = true;
    mutices[index]->unlock();
}


string PortableClient::searchHost() const {
    threads = new thread * [checkedIpCount];
    mutices = new mutex * [checkedIpCount];
    threadFinished = new bool[checkedIpCount];

    string myIP = this->getIP();
    for (int i = 0; i < 2; i++) {
        myIP.pop_back();
    }
    for (int i = 1; i < checkedIpCount; i++) {
        //set i as (possibly 3) last digits of iü
        int prevSize = myIP.size();
        myIP.append(to_string(i));

        WSADATA wsaData;
        ConnectSocket = INVALID_SOCKET;
        
        recvbuf = vector<char>(DEFAULT_BUFLEN);

        recvbuflen = DEFAULT_BUFLEN;


        // Initialize Winsock
        iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0) {
            cout << "Client WSAStartup failed with error: \n" << iResult;
            return "";
        }

        struct addrinfo* result = nullptr;
        struct addrinfo* hints = new addrinfo();

        ZeroMemory(hints, sizeof(*hints));
        hints->ai_family = AF_UNSPEC;
        hints->ai_socktype = SOCK_STREAM;
        hints->ai_protocol = IPPROTO_TCP;

        threadFinished[i] = false;
        mutices[i] = new mutex();
        threads[i] = new std::thread(&testIP, (new string(myIP))->c_str(), result, hints, i);

        freeaddrinfo(result);


        //delete appended numbers
        for (int i = myIP.size(); i > prevSize; i--) {
            myIP.pop_back();
        }
        if (foundIP != "") {
            break;
        }
    }
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(5));
        if (foundIP != "") {
            break;
        }

        //check if finished without connecting
        bool finished = true;
        for (int i = 0; i < checkedIpCount; i++) {
            if (threadFinished[i] == false) {
                finished = false;
            }
        }
        if (finished == true) {
            return "";
        }
    }
    return foundIP;
}


#endif