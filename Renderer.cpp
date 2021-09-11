#include "Renderer.hpp"
#include <iostream>
#include <math.h>
#include "GlobalRecources.hpp"
//Call this-----------------------------------------------------------------------------------------------------------
static int* viewSpace;
static int* viewSpaceLimits;

sf::RenderWindow* Renderer::currentWindow;
void Renderer::init(sf::RenderWindow* window) {
    Renderer::currentWindow = window;
    window->setPosition(sf::Vector2i(-13, -13));//fsr thats left top, good library
}

void Renderer::linkViewSpace(int* io_viewSpace, int* io_viewspaceLimits) {
    viewSpace = io_viewSpace;
    viewSpaceLimits = io_viewspaceLimits;

    GlobalRecources::worldRows = getWorldRows();//Set this here caus mouse pos is always called and im too lazy to make a rendered update func
    GlobalRecources::worldCols = getWorldCols();
}

int Renderer::getWorldCols() {
    return currentWindow->getSize().x + viewSpaceLimits[1];
}

int Renderer::getWorldRows() {
    return currentWindow->getSize().y + viewSpaceLimits[0];
}

//coord conversion-------------------------------------------------------------------------------------------------------

int normalResCols = 1920;
int normalResRows = 1080;

static void fromRowCol(int* ioRow, int* ioCol) {
    auto size = Renderer::currentWindow->getSize();
    float helpRow = (float) *ioRow;
    float helpCol = (float) *ioCol;
    *ioRow = (helpRow / normalResRows) * size.y;
    *ioCol = (helpCol / normalResCols) * size.x;
}

static void fromRowColBounds(int* ioW, int* ioH) {
    auto size = Renderer::currentWindow->getSize();
    float helpW = (float) *ioW;
    float helpH = (float) *ioH;
    *ioW = (helpW / normalResCols) * size.x;
    *ioH = (helpH / normalResRows) * size.y;
}

static void toRowCol(int* ioRow, int* ioCol) {
    auto size = Renderer::currentWindow->getSize();
    float helpRow = (float) *ioRow;
    float helpCol = (float) *ioCol;
    *ioRow = (helpRow / size.y) * normalResRows;
    *ioCol = (helpCol / size.x) * normalResCols;
}

static void toRowColBounds(int* ioW, int* ioH) {
    auto size = Renderer::currentWindow->getSize();
    float helpW = (float) *ioW;
    float helpH = (float) *ioH;
    *ioW = (helpW / normalResCols) * size.x;
    *ioH = (helpH / normalResRows) * size.y;
}
//\coord conversion-------------------------------------------------------------------------------------------------------


//Drawing functions-------------------------------------------------------------------------------------------------------

void Renderer::drawRect(int row, int col, int width, int height, sf::Color c, bool solidWithViewspace) {
    fromRowColBounds(&width, &height);
    fromRowCol(&row, &col);
    sf::RectangleShape* square = new sf::RectangleShape(sf::Vector2f(width, height));

    square->setFillColor(c);
    if (solidWithViewspace == true) {
        square->move(col, row);
    }
    else {
        square->move(col - viewSpace[1], row - viewSpace[0]);
    }
    Renderer::currentWindow->draw(*square);
    
    delete square;
}


void Renderer::drawRectOutline(int row, int col, int width, int height, sf::Color c, int thickness, bool solidWithViewspace) {
    fromRowColBounds(&width, &height);
    fromRowCol(&row, &col);
    int unusedHelp = 0;//we dont need to write 2 values but only have functions for 2 values (lazyness)
    fromRowColBounds(&thickness, &unusedHelp);

    sf::RectangleShape* square = new sf::RectangleShape(sf::Vector2f(width - thickness, height -  2 * thickness));
    square->setOutlineColor(c);
    square->setFillColor(sf::Color(0, 0, 0, 0));
    square->setOutlineThickness(thickness);

    if (solidWithViewspace == true) {
        square->move(col, row);
    }
    else {
        square->move(col - viewSpace[1], row - viewSpace[0]);
    }
    Renderer::currentWindow->draw(*square);
    delete square;
}

void Renderer::drawCircle(int row, int col, int radius, sf::Color c, bool fill, int outlineThickness, bool solidWithViewspace) {
    fromRowCol(&row, &col);
    int unusedHelp = 0;//we dont need to write 2 values but only have functions for 2 values (lazyness)
    fromRowColBounds(&outlineThickness, &unusedHelp);
    fromRowColBounds(&radius, &unusedHelp);

    sf::CircleShape* circle = new sf::CircleShape(radius);
    if (fill == true) {
        circle->setFillColor(c);
    }
    else {
        circle->setOutlineColor(c);
        circle->setOutlineThickness(outlineThickness);
        circle->setFillColor(sf::Color(0, 0, 0, 0));
    }
    if (solidWithViewspace == true) {
        circle->move(col, row);
    }
    else {
        circle->move(col - viewSpace[1], row - viewSpace[0]);
    }
    Renderer::currentWindow->draw(*circle);
    delete circle;
}

void Renderer::drawLine(int row1, int col1, int row2, int col2, sf::Color c, int thickness) {
    fromRowCol(&row1, &col1);
    fromRowCol(&row2, &col2);

    float dCol = col2 - col1;
    float dRow = row2 - row1;
    int ht = thickness / 2;
    float rot = atan2(dRow, dCol) * 57.2958f;
    sf::RectangleShape line = sf::RectangleShape(sf::Vector2f(std::sqrt(std::abs(dCol) * std::abs(dCol) + std::abs(dRow) * std::abs(dRow)), ht * 2));

    line.setOrigin(0, ht);
    line.setPosition(col1 - viewSpace[1], row1 - viewSpace[0]);
    line.setRotation(rot);
    line.setFillColor(c);

    Renderer::currentWindow->draw(line);
}

void Renderer::getMousePos(int* o_rows, int* o_cols, bool factorInViewspace, bool factorInBorders) {
    GlobalRecources::worldRows = getWorldRows();//Set this here caus mouse pos is always called and im too lazy to make a rendered update func
    GlobalRecources::worldCols = getWorldCols();

    auto pos = sf::Mouse::getPosition(*currentWindow);
    int col = pos.x;
    int row = pos.y;
    toRowCol(&row, &col);

    if (factorInBorders == true) {
        if (col < normalResCols && row < normalResRows) {
            if (factorInViewspace == true) {
                *o_cols = col + viewSpace[1];
                *o_rows = row + viewSpace[0];
            }
            else {
                *o_cols = col;
                *o_rows = row;
            }
        }
    }
    else {
        if (factorInViewspace == true) {
            *o_cols = col + viewSpace[1];
            *o_rows = row + viewSpace[0];
        }
        else {
            *o_cols = col;
            *o_rows = row;
        }
    }
}

void Renderer::updateViewSpace() {
    auto size = currentWindow->getSize();

    int moveSpeed = 30;
    
    int mouseCol = -1;
    int mouseRow = -1;
    getMousePos(&mouseRow, &mouseCol, false, true);
    int* helpViewSpace = new int[2];
    helpViewSpace[0] = viewSpace[0];
    helpViewSpace[1] = viewSpace[1];


    int vsLeft = viewSpaceLimits[0];
    int vsRight = viewSpaceLimits[1];
    int vsTop = viewSpaceLimits[2];
    int vsBottom = viewSpaceLimits[3];
    toRowColBounds(&vsTop, &vsLeft);
    toRowColBounds(&vsBottom, &vsRight);
    if (mouseRow != -1) {
        int localRow = mouseRow;
        int localCol = mouseCol;
        fromRowCol(&mouseRow, &mouseCol);

        if (localRow < normalResRows / 10) {
            if (helpViewSpace[0] - moveSpeed > vsTop) {
                helpViewSpace[0] -= moveSpeed;
            }
            else {
                helpViewSpace[0] = vsTop;
            }
        }
        if (localRow > normalResRows * 0.9) {
            if (helpViewSpace[0] + moveSpeed < vsBottom) {
                helpViewSpace[0] += moveSpeed;
            }
            else {
                helpViewSpace[0] = vsBottom;
            }
        }
        if (localCol < normalResCols / 10) {
            if (helpViewSpace[1] - moveSpeed > vsLeft) {
                helpViewSpace[1] -= moveSpeed;
            }
            else {
                helpViewSpace[1] = vsLeft;
            }
        }
        if (localCol > normalResCols * 0.9f) {
            if (helpViewSpace[1] + moveSpeed < vsRight) {
                helpViewSpace[1] += moveSpeed;
            }
            else {
                helpViewSpace[1] = vsRight;
            }
        }
    }
    delete[] viewSpace;
    viewSpace = helpViewSpace;
}


sf::Texture Renderer::loadTexture(std::string path, bool repeat) {
    sf::Texture texture;
    if (!texture.loadFromFile(path))
    {
        std::cout << "failed to load texture of path '" << path << "'";
        std::exit(0);
    }
    if (repeat == true) {
        sf::Image img = texture.copyToImage();
        if (texture.loadFromImage(img) == false) {
            std::cout << "failed to load texture image of path '" << path << "'";
            std::exit(0);
        }

        texture.setRepeated(true);
    }
    return texture;
}


void Renderer::drawRectWithTexture(int row, int col, int width, int height, sf::Texture texture, bool solidWithViewspace) {
    fromRowColBounds(&width, &height);
    fromRowCol(&row, &col);

    sf::RectangleShape* square = new sf::RectangleShape(sf::Vector2f(width, height));
    
    square->setTexture(&texture);
    if (solidWithViewspace == true) {
        square->move(col, row);
    }
    else {
        square->move(col - viewSpace[1], row - viewSpace[0]);
    }

    if (texture.isRepeated() == true) {
        sf::IntRect rect = sf::IntRect(col, row, width, height);
        square->setTextureRect(rect);
    }//else stretch

    Renderer::currentWindow->draw(*square);

    delete square;
}

void Renderer::drawText(std::string i_text, int row, int col, int width, int height, sf::Color color) {
    fromRowColBounds(&width, &height);
    fromRowCol(&row, &col);

    sf::Text text;

    sf::Font font;
    if (!font.loadFromFile("Calibri Regular.ttf"))
    {
        std::cout << "error loading font.";
    }

    // select the font
    text.setFont(font); // font is a sf::Font
    text.setString(i_text);



    text.setFillColor(color);
    text.setCharacterSize(width / 4);

    size_t CharacterSize = text.getCharacterSize();

    std::string String = text.getString().toAnsiString();
    bool bold = (text.getStyle() == sf::Text::Bold);
    size_t MaxHeight = 0;

    for (size_t col = 0; col < text.getString().getSize(); col++)
    {
        sf::Uint32 Character = String.at(col);

        const sf::Glyph& CurrentGlyph = font.getGlyph(Character, CharacterSize, bold);

        size_t Height = CurrentGlyph.bounds.height;

        if (MaxHeight < Height)
            MaxHeight = Height;
    }

    sf::FloatRect rect = text.getGlobalBounds();

    rect.left = ((float)width / 2.0f) - (rect.width / 2.0f);
    rect.top = ((float)height / 2.0f) - ((float)MaxHeight / 2.0f) - (rect.height - MaxHeight) + ((rect.height - CharacterSize) / 2.0f);

    int l = rect.left - (currentWindow->getSize().x / 385);
    int t = rect.top - (currentWindow->getSize().y / 72);

    text.setPosition(l + col, t + row);

   // text.setStyle(sf::Text::Bold | sf::Text::Underlined);
    currentWindow->draw(text);
}