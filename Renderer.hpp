#pragma once

#include <SFML/Graphics.hpp>
class Renderer {
public:
    static sf::RenderWindow* currentWindow;

    static void init(sf::RenderWindow* window);
    static void initGrid( int rows,  int cols);
    static int getWorldRows();
    static int getWorldCols();

    static void drawRect(int row, int col, int width, int height, sf::Color c, bool solidWithViewspace);
    static void drawRectOutline(int row, int col, int width, int height, sf::Color c, int thickness, bool solidWithViewspace);
    static void drawCircle(int row, int col, int radius, sf::Color c, bool fill, int outlineThickness, bool solidWithViewspace);
    static void drawLine(int row1, int col1, int row2, int col2, sf::Color c, int thickness);
  
    static void limitMouse(int row, int col);
    static void getMousePos(int* o_x, int* o_y, bool factorInViewspace, bool factorInBorders);
    static void updateViewSpace();
    static void linkViewSpace(int* io_viewSpace, int* io_viewspaceLimits);

    static void drawRectWithTexture(int row, int col, int width, int height, sf::Texture texture, bool solidWithViewspace);

    static sf::Texture loadTexture(std::string path, bool repeat);

    static void drawText(std::string text, int row, int col, int width, int height, sf::Color color);
};


//help button class
class Button {
public:
    Button(int row, int col, int width, int height, sf::Color color, std::string text, sf::Color textColor) {
        this->row = row;
        this->col = col;
        this->width = width;
        this->height = height;
        this->text = text;
        this->color = color;
        this->textColor = textColor;
    }
    bool sameClick = false;
    bool isPressed() {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) == true && sameClick == false) {
            sameClick = true;
            int mRow;
            int mCol;
            Renderer::getMousePos(&mCol, &mRow, false, true);
            if (mRow > this->row && mRow < this->row + this->height) {
                if (mCol > this->col && mCol < this->col + this->width) {
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
            Renderer::drawRectOutline(row, col, width, height, newColor, 10, true);
        }
        else {
            Renderer::drawRectOutline(row, col, width, height, newColor, 3, true);
        }
        Renderer::drawRect(row, col, width, height, newColor, true);

        Renderer::drawText(text, row, col, width, height, textColor);
    }
private:
    int row, col, width, height;
    sf::Color color, textColor;
    std::string text;
};