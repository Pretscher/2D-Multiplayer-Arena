#pragma once

#include <SFML/Graphics.hpp>
class Renderer {
public:
    static sf::RenderWindow* currentWindow;

    static void init(sf::RenderWindow* window);
    static void initGrid( int rows,  int cols);

    static void drawRect(int row, int col, int width, int height, sf::Color c, bool solidWithViewspace);
    static void drawRectOutline(int row, int col, int width, int height, sf::Color c, int thickness, bool solidWithViewspace);
    static void drawCircle(int row, int col, int radius, sf::Color c, bool fill, bool solidWithViewspace);
    static void drawLine(int row1, int col1, int row2, int col2, sf::Color c);
  
    static void limitMouse(int row, int col);
    static void getMousePos(int* o_x, int* o_y, bool factorInViewSpace);
    static void updateViewSpace();
    static void linkViewSpace(int* io_viewSpace, int* io_viewspaceLimits);

    static void drawRectWithTexture(int row, int col, int width, int height, sf::Texture texture, bool solidWithViewspace);

    static sf::Texture loadTexture(std::string path);

};