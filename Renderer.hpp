#pragma once

#include <SFML/Graphics.hpp>
class Renderer {
public:
    static sf::RenderWindow* currentWindow;

    static void init(sf::RenderWindow* window);
    static void initGrid( int rows,  int cols);

    static void drawRect(float x, float y, float width, float height, sf::Color c);
    static void drawRectOutline(float x, float y, float width, float height, sf::Color c, float thickness);
    static void drawLine(float x1, float y1, float x2, float y2, sf::Color c);
    static void drawCircle(float x, float y, float radius, sf::Color c, bool fill);
    static void drawText(float x, float y, float size, sf::Color c, char const* iText);

    static void drawRectRC(int row, int col, int width, int height, sf::Color c);

    static bool rectsChanged;
    static sf::Vector2f* getMousePos();
};