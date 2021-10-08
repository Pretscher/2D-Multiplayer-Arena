#include "PortableServer.hpp"
#include <thread>
#include "iostream"
#include <vector>
#include <mutex>
#include <stdlib.h>

using namespace std;
/*
static int linClientSocket;
static int server_fd;
static struct sockaddr_in address;
static int addrlen;
static int recvbuflen = 512;

static bool gotNewMessage = false;
static shared_ptr<string> lastMessage(new string());
static shared_ptr<mutex> mtx(new mutex());
static bool waitHandShaking = true;

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
    if ((linClientSocket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        cout << "accept";
        shutdown(linClientSocket, SHUT_RDWR);
    }
    setConnected(true);
}

void PortableServer::receiveMultithreaded() {
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(1));
        //received something
        char* recvbuf = new char[recvbuflen];
        int inputLenght = read(linClientSocket, recvbuf, recvbuflen);
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
            waitHandShaking = false;//sending to client before receiving again
            //connection setup
            if (lastMessage->compare("12345") == 0) {
                sendToClient("12345");//set wait to true again, client has to make the first move
                //waitHandShaking = true;
                lastMessage->clear();
                gotNewMessage = false;
            }
            mtx->unlock();
        }

        if (inputLenght < 0) {
            cout << "Lost connection to client.";
            shutdown(linClientSocket, SHUT_RDWR);
            setConnected(false);
            return;
        }
    }
}

void PortableServer::sendToClient(const char* message) {
    if (waitHandShaking == false) {
        send(linClientSocket, message, strlen(message), 0);
        waitHandShaking = true;
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

*/

PortableServer::PortableServer() {
    wait = true;
    gotNewMessage = false;
    lastMessage = shared_ptr<string>(new string());
#ifdef _WIN32
    winClientSocket = INVALID_SOCKET;
#endif
}

void PortableServer::waitForClient() {
    portableStartup();//make listen socket and stuff ready
    portableConnect();//listen for clients
}



void PortableServer::sendToClient(const char* message) {
    if (wait == false) {
        int iResult = portableSend(message);
        wait = true;
    }
}

void PortableServer::receiveMultithreaded() {
    // Receive until the peer shuts down the connection
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(1));
        char* recvBuffer = new char[recvbuflen];
        int iResult = portableRecv(recvBuffer);

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
            wait = false;
            //connection setup
            if (lastMessage->compare("12345") == 0) {
                sendToClient("12345");//sets wait to false
                wait = false;
                lastMessage->clear();
                gotNewMessage = false;
                setConnected(true);
            }

            mtx->unlock();
        }

        if (iResult < 0) {
            cout << "Lost connection to client.";
            portableShutdown();
            setConnected(false);
            return;
        }
    }
}

shared_ptr<string> PortableServer::getLastMessage() const {
    return lastMessage;
}

shared_ptr<mutex> PortableServer::getMutex() const {
    return mtx;
}


bool PortableServer::newMessage() {
    bool temp = gotNewMessage;
    gotNewMessage = false;
    return temp;
}


string PortableServer::getIP() const {
#ifdef __linux__ 

    struct ifaddrs* ifaddr, * ifa;
    int family, s;
    char host[265];

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }


    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, 265, NULL, 0, 1);

        if ( /*(strcmp(ifa->ifa_name,"wlan0")==0)&&( */ ifa->ifa_addr->sa_family == AF_INET) // )
        {
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            printf("\tInterface : <%s>\n", ifa->ifa_name);
            printf("\t  Address : <%s>\n", host);
        }
    }
    freeifaddrs(ifaddr);
    return string(host);
#elif _WIN32
    char hostname[255];
    struct hostent* he;
    struct in_addr** addr_list;

    WSAData data;
    WSAStartup(MAKEWORD(2, 2), &data);

    gethostname(hostname, 255);

    if ((he = gethostbyname(hostname)) == NULL) {
        std::cout << "gethostbyname error" << std::endl;
        return string();
    }
    else {
        addr_list = (struct in_addr**) he->h_addr_list;
        return string(inet_ntoa(*addr_list[0]));
    }
    return string();
#endif
}

int PortableServer::portableSend(const char* message) const {
#ifdef __linux__ 
    int result = send(linClientSocket, message, strlen(message), 0);
    return result;
#elif _WIN32
    int result = send(winClientSocket, message, (int) strlen(message), 0);
    if (result == SOCKET_ERROR) {
        cout << "Server Message Sending Error: \n" << message;
    }
    return result;
#endif
}

int PortableServer::portableRecv(char* recvBuffer) {
#ifdef __linux__ 
    return read(linClientSocket, recvBuffer, recvbuflen);
#elif _WIN32
    return recv(winClientSocket, recvBuffer, recvbuflen, 0);
#endif
}

void PortableServer::portableConnect() {
#ifdef __linux__ 
    if ((linClientSocket = accept(server_fd, (struct sockaddr*) &address, (socklen_t*) &addrlen)) < 0) {
        cout << "Server accept failed";
    }

#elif _WIN32
    winClientSocket = accept(ListenSocket, nullptr, nullptr);
    if (winClientSocket == INVALID_SOCKET) {
        cout << "Server accept failed with error: \n" << WSAGetLastError();
        closesocket(ListenSocket);
        WSACleanup();
        exit(0);
        return;
    }
#endif
}

void PortableServer::portableShutdown() {
#ifdef __linux__ 
    shutdown(linClientSocket, SHUT_RDWR);
#elif _WIN32
    closesocket(winClientSocket);
#endif
}

//make ready for connecting to a new client (has to be called after connecting to a client aswell)
void PortableServer::portableStartup() {
#ifdef __linux__ 

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
    if (bind(server_fd, (struct sockaddr*) &address,
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
#elif _WIN32
    WSADATA wsaData;
    ListenSocket = INVALID_SOCKET;

    struct addrinfo* result = nullptr;
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
    iResult = bind(ListenSocket, result->ai_addr, (int) result->ai_addrlen);
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
#endif
}