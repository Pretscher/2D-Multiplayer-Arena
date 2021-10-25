#include "PortableServer.hpp"
#include <thread>
#include "iostream"
#include <vector>
#include <mutex>
#include <stdlib.h>

using namespace std;
/*
static int linClientSocket;
static int listenSocket;
static struct sockaddr_in address;
static int addrlen;
static int recvbuflen = 512;

static bool gotNewMessage = false;
static shared_ptr<string> lastMessages(new string());
static shared_ptr<mutex> mtx(new mutex());
static bool waitHandShaking = true;

PortableServer::PortableServer() {

    int opt = 1;
    addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((listenSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        cout << "socket failed";
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
        &opt, sizeof(opt)))
    {
        cout << "setsockopt";
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(listenSocket, (struct sockaddr*)&address,
        sizeof(address)) < 0)
    {
        cout << "bind failed";
        exit(EXIT_FAILURE);
    }
    if (listen(listenSocket, 3) < 0)
    {
        cout << "listen";
        exit(EXIT_FAILURE);
    }
}

void PortableServer::waitForClient() {
    if ((linClientSocket = accept(listenSocket, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
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
            lastMessages->clear();
            gotNewMessage = true;
            //save message
            for (int i = 0; i < inputLenght; i++) {
                lastMessages->push_back(recvbuf[i]);
            }
            delete[] recvbuf;
            waitHandShaking = false;//sending to client before receiving again
            //connection setup
            if (lastMessages->compare("12345") == 0) {
                sendToClient("12345");//set wait to true again, client has to make the first move
                //waitHandShaking = true;
                lastMessages->clear();
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
    return lastMessages;
}

bool PortableServer::isConnected() const {
    return isConnected();
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

void PortableServer::waitForClient() {
    portableConnect();//listen for clients
}



void PortableServer::sendToClient(int index, string message) {
    if (wait[index] == false) {
        int iResult = portableSend(clientSockets[index], message.c_str());
        wait[index] = true;
    }
}

void PortableServer::receiveMultithreaded(int i) {
    // Receive until the peer shuts down the connection
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(1));
        char* recvBuffer = new char[recvbuflen];
        int iResult = portableRecv(clientSockets[i], recvBuffer);
        //save message
        if (iResult > 0) {
            mtx->lock();//lock caus writing and reading message at the same time is not thread safe
            lastMessages[i].clear();
            gotNewMessage[i] = true;
            //save message
            for (int j = 0; j < iResult; j++) {
                lastMessages[i].push_back(recvBuffer[j]);
            }
            delete[] recvBuffer;
            wait[i] = false;
            //connection setup
            if (lastMessages[i].compare("12345") == 0) {
                sendToClient(i, "12345");//sets wait to false
                lastMessages[i].clear();
                gotNewMessage[i] = false;
            }

            mtx->unlock();
        }

        if (iResult < 0) {
            cout << "Lost connection to client.";
            portableShutdown(clientSockets[i]);
            return;
        }
    }
}

vector<string> PortableServer::getLastMessages() const {
    return lastMessages;
}

shared_ptr<mutex> PortableServer::getMutex() const {
    return mtx;
}


bool PortableServer::newMessage(int index) {
    bool temp = gotNewMessage[index];
    gotNewMessage[index] = false;
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
#elif _WIN64
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

#ifdef __linux__ 
int PortableServer::portableSend(int socket, const char* message) const {
    int result = send(socket, message, strlen(message), 0);
    return result;
}

int PortableServer::portableRecv(int socket, char* recvBuffer) {
    return read(socket, recvBuffer, recvbuflen);
}

void PortableServer::portableShutdown(int socket) {
    shutdown(socket, SHUT_RDWR);
}

#elif _WIN64
int PortableServer::portableSend(SOCKET socket, const char* message) const {
    int result = send(socket, message, (int) strlen(message), 0);
    if (result == SOCKET_ERROR) {
        cout << "Server Message Sending Error: \n" << message;
    }
    return result;
}

void PortableServer::portableShutdown(SOCKET socket) {
    closesocket(socket);
}
#endif

int PortableServer::portableRecv(SOCKET socket, char* recvBuffer) {
    return recv(socket, recvBuffer, recvbuflen, 0);
}



void startNewConnect(PortableServer* server) {
    server->portableConnect();
}

vector<thread> connectThreads;
void PortableServer::portableConnect() {
    wait.push_back(true);
    gotNewMessage.push_back(false);
    lastMessages.push_back(string());
#ifdef __linux__ 

    int listenSocket = 0;
    int opt = 1;
    addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((listenSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        cout << "socket failed";
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
        &opt, sizeof(opt)))
    {
        cout << "setsockopt";
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(listenSocket, (struct sockaddr*) &address,
        sizeof(address)) < 0)
    {
        cout << "bind failed";
        exit(EXIT_FAILURE);
    }
    if (listen(listenSocket, 3) < 0)
    {
        cout << "listen";
        exit(EXIT_FAILURE);
    }

    cout << "Server successfully set up.\n";

    int tempClientSocket = accept(listenSocket, (struct sockaddr*) &address, (socklen_t*) &addrlen));
    if (tempClientSocket < 0) {
        cout << "Server accept failed";
    }
    connectedMtx.lock();
    clientSockets.push_back(tempClientSocket);
    connectedMtx.unlock();

    shutdown(listenSocket, SHUT_RDWR);
#elif _WIN64
    WSADATA wsaData;
    SOCKET listenSocket = INVALID_SOCKET;

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
    listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listenSocket == INVALID_SOCKET) {
        cout << "Server socket failed with error: %ld\n" << WSAGetLastError();
        freeaddrinfo(result);
        WSACleanup();
        exit(0);
        return;
    }

    // Setup the TCP listening socket
    iResult = bind(listenSocket, result->ai_addr, (int) result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        cout << "Server bind failed with error: \n" << WSAGetLastError();
        freeaddrinfo(result);
        closesocket(listenSocket);
        WSACleanup();
        exit(0);
        return;
    }

    freeaddrinfo(result);

    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        cout << "Server listen failed with error: \n" << WSAGetLastError();
        closesocket(listenSocket);
        WSACleanup();
        exit(0);
        return;
    }

    cout << "Server successfully set up.\n";

    SOCKET tempClientSocket = accept(listenSocket, nullptr, nullptr);
    if (tempClientSocket == INVALID_SOCKET) {
        cout << "Server accept failed with error: \n" << WSAGetLastError();
        closesocket(listenSocket);
        WSACleanup();
        exit(0);
        return;
    }

    connectedMtx.lock();
    clientSockets.push_back(tempClientSocket);
    connectedMtx.unlock();
    
    closesocket(listenSocket);
    connectThreads.push_back(thread(&startNewConnect, this));
    this->receiveMultithreaded(connectThreads.size() - 1);//cant be called before pushing back cus loop so -1
#endif
}
