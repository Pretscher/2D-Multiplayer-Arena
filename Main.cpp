#include "SFML/Graphics.hpp"
#include <iostream>
#include <thread>
#include "Renderer.hpp"
#include "Eventhandling.hpp"

sf::RenderWindow* cWindow;

void initDrawing() {

    sf::Event* event = new sf::Event();
    cWindow->setFramerateLimit(60);
    while (cWindow->isOpen()) {
        //everytime a variable you use in here is changed, please log the mutex in the thread that changes the variable.
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
}
int main() {
    cWindow = new sf::RenderWindow(sf::VideoMode(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height), "Mobatemplate", sf::Style::Titlebar | sf::Style::Close);
    Renderer::init(cWindow);

    eventhandling::init();
    initDrawing();
    return 0;
}
