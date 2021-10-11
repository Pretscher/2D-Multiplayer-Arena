#include "PortableClient.hpp"
#include <iostream>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
using namespace std;
/*
#ifdef __linux__ 


#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <mutex>
#include <thread>
#include <vector>

#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

#define PORT 8080

static bool connected = false;
static mutex connectedMtx;
static void setConnected(bool c) {
    connectedMtx.lock();
    connected = c;
    connectedMtx.unlock();
}
static bool isConnected() {
    connectedMtx.lock();
    bool temp = connected;
    connectedMtx.unlock();
    return connected;
}


static int serverSocket;
static int server_fd;
static int recvbuflen = 512;
static vector<int> ConnectSockets;

static bool gotNewMessage = false;
static shared_ptr<string> lastMessage(new string());
static shared_ptr<mutex> writingMessage(new mutex());
static bool waitHandShaking = false;
mutex avHostsMtx;

vector<string> avHosts;
void PortableClient::pushToAvailableHosts(string s) {
    avHostsMtx.lock();
    avHosts.push_back(s);
    avHostsMtx.unlock();
}

vector<string> PortableClient::getAvailableHosts() {
    avHostsMtx.lock();
    vector<string> copy = avHosts;
    avHostsMtx.unlock();
    return std::move(copy);
}

PortableClient::PortableClient() {

}

void PortableClient::connectToHost(string ip) {
    auto copy = avHosts;
    for (int i = 0; i < avHosts.size(); i++) {
        if (avHosts.at(i).compare(ip) == 0) {
            serverSocket = ConnectSockets.at(i);//same index, pushed back simultanioisly
        }
    }
    ConnectSockets.clear();
    setConnected(true);
    cout << "Client successfully connected to server!\n";
}

void PortableClient::receiveMultithreaded() {
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(1));
        char* recvbuf = new char[recvbuflen];
        int inputLenght = read(serverSocket, recvbuf, recvbuflen);
        //received a valid message
        if (inputLenght > 0) {
            writingMessage->lock();
            lastMessage->clear();
            gotNewMessage = true;
            //save message
            for (int i = 0; i < inputLenght; i++) {
                lastMessage->push_back(recvbuf[i]);
            }
            writingMessage->unlock();
            waitHandShaking = false;
        }
        delete[] recvbuf;

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
    return isConnected();
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

#include <ifaddrs.h>
#include <netdb.h>

string PortableClient::getIP() const {
    struct ifaddrs *ifaddr, *ifa;
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

        s=getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in),host, 265, NULL, 0, 1);

        if(ifa->ifa_addr->sa_family==AF_INET){
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            printf("\tInterface : <%s>\n",ifa->ifa_name );
            printf("\t  Address : <%s>\n", host);
        }
    }

    freeifaddrs(ifaddr);
    return string(host);
}


thread** threads;
mutex** mutices;
bool* threadFinished;
string foundIP = "";
const unsigned int checkedIpCount = 128;

void testIP(string serverIP, int index) {
    threadFinished[index] = false;
    mutices[index] = new mutex();
    int tempConnectSocket;
        // Attempt to connect to an address until one succeeds
        // Create a SOCKET for connecting to server
     if ((tempConnectSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        exit(0);
    }
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
     // Convert IPv4 and IPv6 addresses from text to binary form
    inet_pton(AF_INET, serverIP.c_str(), &serv_addr.sin_addr);
    if (connect(tempConnectSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        //std::cout << serverIP  << "\n";
        //printf("\n Connection timed out \n");

    }
    else {
        send(tempConnectSocket, "12345", strlen("12345"), 0);

        long long startTime = std::chrono::system_clock::now().time_since_epoch().count();
        while (std::chrono::system_clock::now().time_since_epoch().count() - startTime < 1000) {
            this_thread::sleep_for(chrono::milliseconds(5));

            char* recvbuf = new char[recvbuflen];
            int inputLenght = read(tempConnectSocket, recvbuf, recvbuflen);

            if (inputLenght > 0) {
                string msg;
                //save message
                for (int i = 0; i < inputLenght; i++) {
                    msg.push_back(recvbuf[i]);
                }
                delete[] recvbuf;
                //connection setup
                if (msg.compare("12345") == 0) {
                    ConnectSockets.push_back(std::move(tempConnectSocket));
                    foundIP = serverIP;//this will be pushed back to string. 
                }
                break;//dont set threadFinished to true so that no multithreading error can occur where the filled string is ignored
            }
            // cout << "Client received message: " << *lastMessage;
            if (inputLenght < 0) {
                cout << "Client recv failed with error: \n";
                exit(0);                
            }
        }
    }
    threadFinished[index] = true;
}


void searchHostsMultiThreaded(PortableClient* client) {
    threads = new thread * [checkedIpCount];
    mutices = new mutex * [checkedIpCount];
    threadFinished = new bool[checkedIpCount];

    string myIP = client->getIP();
    for (int i = 0; i < 2; i++) {
        myIP.pop_back();
    }
    for (int i = 1; i < checkedIpCount; i++) {
        //set i as (possibly 3) last digits of i�
        int prevSize = myIP.size();
        myIP.append(to_string(i));
        string copy = myIP;
        threads[i] = new std::thread(&testIP, std::move(copy), i);

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
            client->pushToAvailableHosts(std::move(foundIP));
            foundIP = "";
        }

        //check if finished without connecting
        bool finished = true;
        for (int i = 0; i < checkedIpCount; i++) {
            if (threadFinished[i] == false) {
                finished = false;
            }
        }
        if (finished == true) {
            return;
        }
    }
    delete[] mutices;
    delete[] threads;
}

thread* searchingHosts;
void PortableClient::searchHosts() {
     searchingHosts = new std::thread(&searchHostsMultiThreaded, this);
}

#elif _WIN32
*/

const int recvbuflen = 512;
shared_ptr<mutex> mtx = shared_ptr<mutex>(new mutex());
PortableClient::PortableClient() {
#ifdef _WIN32
    WSADATA wsaData;
    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cout << "Client WSAStartup failed with error: \n" << iResult;
        return;
    }
#endif
    lastMessage = shared_ptr<string>(new string);
    wait = false;
}

void PortableClient::connectToHost(string ip) {
    auto copy = avHosts;
    for (int i = 0; i < copy.size(); i++) {
        if (copy.at(i).compare(ip) == 0) {
            serverSocket = connectSockets.at(i);//same index, pushed back simultanioisly
        }
        else {
            portableShutdown(connectSockets.at(i));
        }
    }
    connectSockets.clear();
    setConnected(true);
    cout << "Client successfully connected to server!\n";
}

void PortableClient::sendToServer(string message) {
    if (wait == false) {
        portableSend(serverSocket, message.c_str());
        wait = true;
    }
}

void PortableClient::receiveMultithreaded() {
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(1));
        char* recvbuf = new char[recvbuflen];
        int inputLenght = portableRecv(serverSocket, recvbuf);
        //received a valid message
        if (inputLenght > 0) {
            mtx->lock();
            lastMessage->clear();
            gotNewMessage = true;
            //save message
            for (int i = 0; i < inputLenght; i++) {
                lastMessage->push_back(recvbuf[i]);
            }
            mtx->unlock();
            wait = false;
        }
        delete[] recvbuf;
        if (inputLenght < 0) {
            cout << "error, client received message with negative lenght";
            exit(0);
        }
    }
    portableShutdown(serverSocket);
}

shared_ptr<string> PortableClient::getLastMessage() const {
    return lastMessage;
}

shared_ptr<mutex> PortableClient::getMutex() const {
    return mtx;
}

bool PortableClient::isConnected() const {
    return isConnected();
}

bool PortableClient::newMessage() {
    bool temp = gotNewMessage;
    gotNewMessage = false;
    return temp;
}

string PortableClient::getIP() const {
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
    return string();//never reaches this
#endif
  
}


thread** threads;
mutex** mutices;
bool* threadFinished;
string foundIP = "";
const unsigned int checkedIpCount = 128;

void testIP(const char* myIP, int index, PortableClient* client) {
    threadFinished[index] = false;
    mutices[index] = new mutex();

    bool connectSuccess = false;
    //tempConnectSocket has different data type depending on OS so uhm... this code is awful but it does the job
#ifdef __linux__ 
    int tempConnectSocket = client->portableConnect(myIP);
    if (tempConnectSocket >= 0) connectSuccess = true;
#elif _WIN64
    SOCKET tempConnectSocket = client->portableConnect(myIP);
    if (tempConnectSocket != INVALID_SOCKET) connectSuccess = true;
#endif
    if(connectSuccess == true) {
        client->portableSend(tempConnectSocket, "12345");

        long long startTime = std::chrono::system_clock::now().time_since_epoch().count();
        while (std::chrono::system_clock::now().time_since_epoch().count() - startTime < 200) {
            this_thread::sleep_for(chrono::milliseconds(5));
            char* recvBuf = new char[recvbuflen];
            int res = client->portableRecv(tempConnectSocket, recvBuf);
            string msg;
            //save message
            for (int i = 0; i < res; i++) {
                msg.push_back(recvBuf[i]);
            }
            delete[] recvBuf;

            if (msg.compare("12345") == 0) {
                client->connectSockets.push_back(std::move(tempConnectSocket));
                foundIP = myIP;//this will be pushed back to string. 
                break;//dont set threadFinished to true so that no multithreading error can occur where the filled string is ignored
            }
        }
    }
    threadFinished[index] = true;
}


void searchHostsMultiThreaded(PortableClient* client) {
    threads = new thread * [checkedIpCount];
    mutices = new mutex * [checkedIpCount];
    threadFinished = new bool[checkedIpCount];

    string myIP = client->getIP();
    for (int i = 0; i < 2; i++) {
        myIP.pop_back();
    }
    for (int i = 1; i < checkedIpCount; i++) {
        //set i as (possibly 3) last digits of i�
        int prevSize = myIP.size();
        myIP.append(to_string(i));

        threads[i] = new std::thread(&testIP, (new string(myIP))->c_str(), i, client);

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
            client->pushToAvailableHosts(std::move(foundIP));
            foundIP = "";
        }

        //check if finished without connecting
        bool finished = true;
        for (int i = 0; i < checkedIpCount; i++) {
            if (threadFinished[i] == false) {
                finished = false;
            }
        }
        if (finished == true) {
            return;
        }
    }
    delete[] mutices;
    delete[] threads;
}


thread* searchingHosts;
void PortableClient::searchHosts() {
     searchingHosts = new std::thread(&searchHostsMultiThreaded, this);
}

#ifdef __linux__ 
int PortableClient::portableSend(int socket, const char* message) const {
    int result = send(socket, message, strlen(message), 0);
    return result;
}

int PortableClient::portableConnect(const char* connectIP) {
    int listenSocket;
    // Attempt to connect to an address until one succeeds
    // Create a SOCKET for connecting to server
    if ((listenSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        exit(0);
    }
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    // Convert IPv4 and IPv6 addresses from text to binary form
    inet_pton(AF_INET, connectIP, &serv_addr.sin_addr);
    if (connect(listenSocket, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) >= 0) {
        return listenSocket;
    }
    return -1;
}
void PortableClient::portableShutdown(int socket) {
    shutdown(socket, SHUT_RDWR);
}


int PortableClient::portableRecv(int socket, char* recvBuffer) {
    return read(socket, recvBuffer, recvbuflen);
}

#elif _WIN64

void PortableClient::portableShutdown(SOCKET& socket) {
    // shutdown the connection since we're done
    int iResult = shutdown(socket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        cout << "Client shutdown failed with error: \n" << WSAGetLastError();
        closesocket(socket);
        WSACleanup();
        exit(0);
        return;
    }
    // cleanup
    closesocket(socket);
    WSACleanup();
}

int PortableClient::portableSend(SOCKET& socket, const char* message) const {
    int result = send(socket, message, (int) strlen(message), 0);
    if (result == SOCKET_ERROR) {
        cout << "Client Message Sending Error: \n" << message;
    }
    return result;
}

int PortableClient::portableRecv(SOCKET& socket, char* recvBuffer) {
    return recv(socket, recvBuffer, recvbuflen, 0);
}

SOCKET PortableClient::portableConnect(const char* connectIP) {
    struct addrinfo* result = nullptr;
    struct addrinfo* hints = new addrinfo();

    int res = getaddrinfo(connectIP, port.c_str(), hints, &result);
    if (res != 0) {
        cout << "Client getaddrinfo failed with error: \n";
        WSACleanup();
        return INVALID_SOCKET;
    }
    if (result != nullptr) {
        ZeroMemory(hints, sizeof(*hints));
        hints->ai_family = AF_UNSPEC;
        hints->ai_socktype = SOCK_STREAM;
        hints->ai_protocol = IPPROTO_TCP;

        // Create a SOCKET for connecting to server
        SOCKET listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (listenSocket == INVALID_SOCKET) {
            cout << "Client socket connection failed with error: %ld\n" << WSAGetLastError();
            WSACleanup();
            return INVALID_SOCKET;
        }

        res = connect(listenSocket, result->ai_addr, (int) result->ai_addrlen);
        if (res == SOCKET_ERROR) {
            closesocket(listenSocket);
            return INVALID_SOCKET;
        }
        freeaddrinfo(result);
        delete hints;
        return listenSocket;
    }
    return INVALID_SOCKET;
}
#endif
