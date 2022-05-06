#include "SFML/Graphics.hpp"
#include "iostream" 
using namespace std;
#include <thread>
#include "Renderer.hpp"
#include "Eventhandling.hpp"
#include <memory>

void initDrawing(shared_ptr<sf::RenderWindow> cWindow, Eventhandling eventhandling) {

    sf::Event events;

    cWindow->setFramerateLimit(60);
    while (cWindow->isOpen()) {
        //everytime a variable you use in here is changed, please log the mutex in the thread that changes the variable.
        while (cWindow->pollEvent(events)) {
            if (events.type == sf::Event::Closed) {
                cWindow->close();
                exit(0);
            }
        }
        cWindow->clear();//clear with every iteration

        //draw here-------------------------------------------------------
        eventhandling.eventloop();
        eventhandling.drawingloop();

        //\draw here------------------------------------------------------
        cWindow->display();//display things drawn since clear() was called

    }
}

int main() {

    shared_ptr<sf::RenderWindow> cWindow = shared_ptr<sf::RenderWindow>(new sf::RenderWindow(sf::VideoMode(sf::VideoMode::getDesktopMode().width,
                                                sf::VideoMode::getDesktopMode().height - 150), "Mobatemplate", sf::Style::Titlebar | sf::Style::Close));
    Renderer::init(cWindow);
    Eventhandling eventhandling;
    initDrawing(std::move(cWindow), std::move(eventhandling));
    return 0;
}
