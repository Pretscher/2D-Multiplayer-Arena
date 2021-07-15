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

void fromCartesianCoords(float* ioX, float* ioY) {
    auto size = Renderer::currentWindow->getSize();
    *ioX = (float)(((*ioX + 1.0f) / 2.0f) * ((float)firstSize.x));
    *ioY = (float)(((1.0f - *ioY) / 2.0f) * ((float)firstSize.y));
}

void toCartesianCoords(float* ioX, float* ioY) {
    auto size = Renderer::currentWindow->getSize();
    *ioX = (float)((*ioX * 2.0f / (float)firstSize.x) - 1.0f);
    *ioY = (float)((-*ioY * 2.0f / ((float)firstSize.y)) + 1.0f);
}

void fromCartesianBounds(float* ioX, float* ioY) {
    auto size = Renderer::currentWindow->getSize();
    *ioX = (int)((*ioX / 2.0f) * ((float)firstSize.x));
    *ioY = (int)((*ioY / 2.0f) * ((float)firstSize.y));
}

void toCartesianBounds(float* ioX, float* ioY) {
    auto size = Renderer::currentWindow->getSize();
    *ioX = (int)((*ioX * 2.0f) / ((float)firstSize.x));
    *ioY = (int)((*ioY * 2.0f) / ((float)firstSize.y));
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



void Renderer::drawRectC(float ioX, float ioY, float width, float height, sf::Color c) {
    fromCartesianBounds(&width, &height);
    sf::RectangleShape* square = new sf::RectangleShape(sf::Vector2f(width, height));

    square->setFillColor(c);
    fromCartesianCoords(&ioX, &ioY);
    square->move(ioX, ioY);
    Renderer::currentWindow->draw(*square);
    delete square;
}

void Renderer::drawRectOutlineC(float ioX, float ioY, float width, float height, sf::Color c, float thickness) {
    float help = 0;
    thickness *= 1000;//for better values, realistic between 1 and 10
    toCartesianBounds(&thickness, &help);

    fromCartesianBounds(&width, &height);
    sf::RectangleShape* square = new sf::RectangleShape(sf::Vector2f(width - thickness, height - thickness));
    square->setOutlineColor(c);
    square->setFillColor(sf::Color(0, 0, 0, 0));
    square->setOutlineThickness(thickness);


    fromCartesianCoords(&ioX, &ioY);
    square->move(ioX + thickness, ioY);
    Renderer::currentWindow->draw(*square);
    delete square;
}

void Renderer::drawCircleC(float ioX, float ioY, float radius, sf::Color c, bool fill) {
    float unusedHelp = 0;
    fromCartesianBounds(&radius, &unusedHelp);
    sf::CircleShape* circle = new sf::CircleShape(radius);
    circle->setOutlineColor(c);
    if (fill == true) {
        circle->setFillColor(c);
    }
    else {
        circle->setFillColor(sf::Color(0, 0, 0, 0));
    }
    fromCartesianCoords(&ioX, &ioY);
    circle->move(ioX, ioY);
    Renderer::currentWindow->draw(*circle);
    delete circle;
}

void Renderer::drawLineC(float x1, float y1, float x2, float y2, sf::Color c) {
    fromCartesianCoords(&x1, &y1);
    fromCartesianCoords(&x2, &y2);
    sf::Vertex line[] =
    {
        sf::Vertex(sf::Vector2f(x1, y1)),
        sf::Vertex(sf::Vector2f(x2, y2))
    };
    
    line->color = c;
    Renderer::currentWindow->draw(line, 2, sf::Lines);
}

//Coordinate System Math----------------------------------------------------------------------------------------

sf::Vector2f* Renderer::getMousePos() {
    auto pos = sf::Mouse::getPosition();
    auto offset = currentWindow->getPosition();
    pos.x -= offset.x;
    pos.y -= offset.y;
    float retX = pos.x;
    float retY = pos.y;
    toCartesianCoords(&retX, &retY);
    return new sf::Vector2f(retX, retY);
}

void Renderer::drawText(float x, float y, float size, sf::Color c, char const* iText) {
    fromCartesianCoords(&x, &y);

    sf::Text* text = new sf::Text();
    sf::Font* font = new sf::Font();
    font->loadFromFile("consolas.ttf");
    text->setFont(*font);
    
    float help = 0;
    size *= 10000;//for better values, realistic between 1 and 10
    toCartesianBounds(&size, &help);

    text->setCharacterSize(size);
    text->setFillColor(c);
    text->setPosition(x, y);
    text->setString(iText);
    //text.setStyle(sf::Text::Bold | sf::Text::Underlined);
    currentWindow->draw(*text);
    delete text;
    delete font;
}