#include "SFML/Graphics.hpp"
#include <iostream>
#include <thread>
#include "Renderer.hpp"
#include "Eventhandling.hpp"
#include "Server.hpp"
#include "Client.hpp"

sf::RenderWindow* cWindow;

void initDrawing() {

    sf::Event* event = new sf::Event();
    while (cWindow->isOpen()) {
        //everytime a variable you use in here is changed, please log the mutex in the thread that changes the variable.
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        while (cWindow->pollEvent(*event)) {
            if (event->type == sf::Event::Closed) {
                cWindow->close();
                std::exit(0);
            }
        }
        cWindow->clear();//clear with every iteration

        //draw here-------------------------------------------------------
        eventhandling::eventloop();
        eventhandling::drawingloop();

        //\draw here------------------------------------------------------
        cWindow->display();//display things drawn since clear() was called

    }
    //doesnt reach this before programm is ending, but i heard its good style
    delete cWindow;
    delete event;

    std::exit(0);
}

std::thread* nThread;


void initServer() {
    Server* server = new Server();
    server->startConnection();
}

int main() {
    nThread = new std::thread(&initServer);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::string s = "192.168.178.28";
    const char* c = s.c_str();
    Client* client = new Client(c);
    client->sendToServer();
    client->disconnect();



    /*
    cWindow = new sf::RenderWindow(sf::VideoMode(1920, 1080), "MORD!");
    Renderer::init(cWindow);

    eventhandling::init();
    initDrawing();
    */
    return 0;
}
