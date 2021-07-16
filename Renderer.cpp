#include "Renderer.hpp"
#include <iostream>

//Call this-----------------------------------------------------------------------------------------------------------
sf::RenderWindow* Renderer::currentWindow;
sf::Vector2u firstSize;
void Renderer::init(sf::RenderWindow* window) {
    Renderer::currentWindow = window;
    window->setPosition(sf::Vector2i(0, 0));

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    firstSize = Renderer::currentWindow->getSize();
}

 int maxRows, maxCols;
void Renderer::initGrid(int rows, int cols) {
    maxRows = rows;
    maxCols = cols;
}

//coord conversion-------------------------------------------------------------------------------------------------------

void fromRowCol(int* ioRow, int* ioCol) {
    float helpRow = (float)*ioRow;
    float helpCol = (float)*ioCol;
    *ioRow = (helpRow / maxRows) * firstSize.y;
    *ioCol = (helpCol / maxCols) * firstSize.x;
}

void fromRowColBounds(int* ioW, int* ioH) {
    auto size = Renderer::currentWindow->getSize();
    float helpW = (float)*ioW;
    float helpH = (float)*ioH;
    *ioW = (helpW / maxCols) * firstSize.y;
    *ioH = (helpH / maxRows) * firstSize.x;
}

void toRowCol(int* io_X, int* io_Y) {
    auto size = Renderer::currentWindow->getSize();
    float helpRow = (float)*io_X;
    float helpCol = (float)*io_Y;
    *io_X = (helpRow / firstSize.y) * maxRows;
    *io_Y = (helpCol / firstSize.x) * maxCols;
}

//\coord conversion-------------------------------------------------------------------------------------------------------

//Drawing functions-------------------------------------------------------------------------------------------------------

void Renderer::drawRect(int row, int col, int width, int height, sf::Color c) {
    fromRowColBounds(&width, &height);
    sf::RectangleShape* square = new sf::RectangleShape(sf::Vector2f(width, height));

    square->setFillColor(c);
    fromRowCol(&row, &col);
    square->move(col, row);
    Renderer::currentWindow->draw(*square);
    delete square;
}

void Renderer::drawRectOutline(int row, int col, int width, int height, sf::Color c, int thickness) {
    int unusedHelp = 0;
    fromRowColBounds(&thickness, &unusedHelp);

    fromRowColBounds(&width, &height);
    sf::RectangleShape* square = new sf::RectangleShape(sf::Vector2f(width - thickness, height -  2 * thickness));
    square->setOutlineColor(c);
    square->setFillColor(sf::Color(0, 0, 0, 0));
    square->setOutlineThickness(thickness);


    fromRowCol(&row, &col);
    square->move(col + thickness, row + thickness);
    Renderer::currentWindow->draw(*square);
    delete square;
}

void Renderer::drawCircle(int row, int col, int radius, sf::Color c, bool fill) {
    int unusedHelp = 0;
    fromRowColBounds(&radius, &unusedHelp);
    sf::CircleShape* circle = new sf::CircleShape(radius);
    circle->setOutlineColor(c);
    if (fill == true) {
        circle->setFillColor(c);
    }
    else {
        circle->setFillColor(sf::Color(0, 0, 0, 0));
    }
    fromRowCol(&row, &col);
    circle->move(col, row);
    Renderer::currentWindow->draw(*circle);
    delete circle;
}

void Renderer::drawLine(int row1, int col1, int row2, int col2, sf::Color c) {
    fromRowCol(&row1, &col1);
    fromRowCol(&row2, &col2);
    sf::Vertex line[] =
    {
        sf::Vertex(sf::Vector2f(col1, row1)),
        sf::Vertex(sf::Vector2f(col2, row2))
    };

    line->color = c;
    Renderer::currentWindow->draw(line, 2, sf::Lines);
}

void Renderer::getMousePos(int* o_x, int* o_y) {
    auto pos = sf::Mouse::getPosition();
    auto offset = currentWindow->getPosition();
    pos.x -= offset.x;
    pos.y -= offset.y;
    *o_x = pos.x;
    *o_y = pos.y;
    toRowCol(o_x, o_y);
    fromRowCol(o_x, o_y);
}
