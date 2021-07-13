#include "Renderer.hpp"
#include <iostream>

//Call this-----------------------------------------------------------------------------------------------------------
sf::RenderWindow* Renderer::currentWindow;

void Renderer::init(sf::RenderWindow* window) {
    Renderer::currentWindow = window;
    window->setPosition(sf::Vector2i(0, 0));
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
}

 int maxRows, maxCols;
void Renderer::initGrid( int rows,  int cols) {
    maxRows = rows;
    maxCols = cols;
}

//coord conversion-------------------------------------------------------------------------------------------------------

void fromRowCol(int* row, int* col) {
    auto size = Renderer::currentWindow->getSize();
    float helpRow = (float)*row;
    float helpCol = (float)*col;
    *col = (int)((helpCol/ maxCols) * ((float)size.x));
    *row = (int)((helpRow / maxRows) * ((float)size.y));
}

void fromRowColBounds(int* w, int* h) {
    auto size = Renderer::currentWindow->getSize();
    float helpW = (float)*w;
    float helpH = (float)*h;
    *w = (int)((helpW / maxCols) * ((float)size.x));
    *h = (int)((helpH / maxRows) * ((float)size.y));
}

void fromCartesianCoords(float* ioX, float* ioY) {
    auto size = Renderer::currentWindow->getSize();
    *ioX = (float)(((*ioX + 1.0f) / 2.0f) * ((float)size.x));
    *ioY = (float)(((1.0f - *ioY) / 2.0f) * ((float)size.y));
}

void toCartesianCoords(float* ioX, float* ioY) {
    auto size = Renderer::currentWindow->getSize();
    *ioX = (float)((*ioX * 2.0f / (float)size.x) - 1.0f);
    *ioY = (float)((-*ioY * 2.0f / ((float)size.y)) + 1.0f);
}

void fromCartesianBounds(float* ioX, float* ioY) {
    auto size = Renderer::currentWindow->getSize();
    *ioX = (int)((*ioX / 2.0f) * ((float)size.x));
    *ioY = (int)((*ioY / 2.0f) * ((float)size.y));
}

void toCartesianBounds(float* ioX, float* ioY) {
    auto size = Renderer::currentWindow->getSize();
    *ioX = (int)((*ioX * 2.0f) / ((float)size.x));
    *ioY = (int)((*ioY * 2.0f) / ((float)size.y));
}

//\coord conversion-------------------------------------------------------------------------------------------------------

//Drawing functions----------------------------------------------------------------------------------------------------

void Renderer::drawRectRC(int row, int col, int width, int height, sf::Color c) {
    fromRowColBounds(&width, &height);
    sf::RectangleShape* square = new sf::RectangleShape(sf::Vector2f(width, height));

    square->setFillColor(c);
    fromRowCol(&row, &col);
    square->move(row, col);
    Renderer::currentWindow->draw(*square);
    delete square;
}
/*
void Renderer::drawRectOutline(float ioX, float ioY, float width, float height, sf::Color c, float thickness) {
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

void Renderer::drawCircle(float ioX, float ioY, float radius, sf::Color c, bool fill) {
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

void Renderer::drawLine(float x1, float y1, float x2, float y2, sf::Color c) {
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

*/





void Renderer::drawRect(float ioX, float ioY, float width, float height, sf::Color c) {
    fromCartesianBounds(&width, &height);
    sf::RectangleShape* square = new sf::RectangleShape(sf::Vector2f(width, height));

    square->setFillColor(c);
    fromCartesianCoords(&ioX, &ioY);
    square->move(ioX, ioY);
    Renderer::currentWindow->draw(*square);
    delete square;
}

void Renderer::drawRectOutline(float ioX, float ioY, float width, float height, sf::Color c, float thickness) {
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

void Renderer::drawCircle(float ioX, float ioY, float radius, sf::Color c, bool fill) {
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

void Renderer::drawLine(float x1, float y1, float x2, float y2, sf::Color c) {
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