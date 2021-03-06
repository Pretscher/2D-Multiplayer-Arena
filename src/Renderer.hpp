#pragma once
#include <iostream>
#include <memory>
using namespace std;
#include <SFML/Graphics.hpp>
class Renderer {
public:
    static shared_ptr<sf::RenderWindow> currentWindow;

    static void init(shared_ptr<sf::RenderWindow> window);

    static int getWorldYs();
    static int getWorldXs();

    static void drawRect(int y, int x, int width, int height, sf::Color c, bool solidWithViewspace);
    static void drawRectOutline(int y, int x, int width, int height, sf::Color c, int thickness, bool solidWithViewspace);
    static void drawCircle(int y, int x, int radius, sf::Color c, bool fill, int outlineThickness, bool solidWithViewspace);
    static void drawLine(int y1, int x1, int y2, int x2, sf::Color c, int thickness);
  
    static void getMousePos(int& o_xs, int& o_ys, bool factorInViewspace, bool factorInBorders);
    static void updateViewSpace();
    static void linkViewSpace(shared_ptr<int[]> io_viewSpace, shared_ptr<const int[]> io_viewspaceLimits);

    static void drawRectWithTexture(int y, int x, int width, int height, sf::Texture texture, bool solidWithViewspace);

    static sf::Texture loadTexture(string path, bool repeat);

    static void drawText(string text, int y, int x, int width, int height, int charSize, sf::Color color);

    static shared_ptr<int[]> viewSpace;
    static shared_ptr<const int[]> viewSpaceLimits;
};


//help button class
class Button {
public:
    Button(int y, int x, int width, int height, sf::Color color, string text, int charSize, sf::Color textColor) {
        this->y = y;
        this->x = x;
        this->width = width;
        this->height = height;
        this->text = text;
        this->color = color;
        this->textColor = textColor;
        this->charSize = charSize;
    }
    bool sameClick = false;
    bool isPressed() {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) == true && sameClick == false) {
            sameClick = true;
            int mX, mY;
            Renderer::getMousePos(mX, mY, false, true);
            if (mY > this->y && mY < this->y + this->height) {
                if (mX > this->x && mX < this->x + this->width) {
                    return true;
                }
            }
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) == false) {
            sameClick = false;
        }
        return false;
    }

    void draw() {
        sf::Color newColor = color;
        if (this->isPressed() == true) {
            newColor.r += 50;
            newColor.g += 50;
            newColor.b += 50;
            Renderer::drawRectOutline(x, y, width, height, newColor, 3, true);
        }
        else {
            Renderer::drawRectOutline(x, y, width, height, newColor, 3, true);
        }
        Renderer::drawRect(x, y, width, height, newColor, true);

        Renderer::drawText(text, x, y, width, height, charSize, textColor);
    }
private:
    int charSize;
    int x, y, width, height;
    sf::Color color, textColor;
    string text;
};