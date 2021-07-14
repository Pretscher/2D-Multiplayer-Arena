#pragma once

#include <SFML/Graphics.hpp>
class Renderer {
public:
    static sf::RenderWindow* currentWindow;

    static void init(sf::RenderWindow* window);
    static void initGrid( int rows,  int cols);

    static void drawRect(int row, int col, int width, int height, sf::Color c);
    static void drawRectOutline(int row, int col, int width, int height, sf::Color c, int thickness);
    static void drawCircle(int row, int col, int radius, sf::Color c, bool fill);
    static void drawLine(int row1, int col1, int row2, int col2, sf::Color c);
   /*
    static void drawRectC(float ioX, float ioY, float width, float height, sf::Color c);
    static void drawRectOutlineC(float ioX, float ioY, float width, float height, sf::Color c, float thickness);
    static void drawCircleC(float ioX, float ioY, float radius, sf::Color c, bool fill);
    static  void drawLineC(float x1, float y1, float x2, float y2, sf::Color c);
    */
    void drawText(float x, float y, float size, sf::Color c, char const* iText);
    static sf::Vector2f* getMousePos();
};