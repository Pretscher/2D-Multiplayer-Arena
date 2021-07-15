#include "SFML/Graphics.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include "Renderer.hpp"

//test

float x, y;

void eventLoop() {
    bool left = false;
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        if (x > 0.5f) left = true;
        if (x < -0.5f) left = false;
        if (left == false) x += 0.01f;
        else x -= 0.01f;
    }
}

void drawingLoop() {
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    sf::RenderWindow* cWindow = new sf::RenderWindow(sf::VideoMode(1920, 1080), "MORD!");
    Renderer::init(cWindow);
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


        //\draw here------------------------------------------------------
        cWindow->display();//display things drawn since clear() was called

    }
    //doesnt reach this before programm is ending, but i heard its good style
    delete cWindow;
    delete event;

    std::exit(0);
}


void init() {
    x = -0.5f;
    y = 0.0f;
    Renderer::initGrid(1920, 1080);
    std::thread* rThread = new std::thread(&drawingLoop);
    eventLoop();
}

int main() {
    init();
    return 0;
}
