#include "Renderer.hpp"
#include <iostream>

//Call this-----------------------------------------------------------------------------------------------------------
static int* viewSpace;
static int* viewSpaceLimits;
sf::RenderWindow* Renderer::currentWindow;
static sf::Vector2u firstSize;
void Renderer::init(sf::RenderWindow* window) {
    Renderer::currentWindow = window;
    window->setPosition(sf::Vector2i(-13, -13));//fsr thats left top, good library

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    firstSize = Renderer::currentWindow->getSize();
}

 int maxRows, maxCols;
void Renderer::initGrid(int rows, int cols) {
    maxRows = rows;
    maxCols = cols;
}

void Renderer::linkViewSpace(int* io_viewSpace, int* io_viewspaceLimits) {
    viewSpace = io_viewSpace;
    viewSpaceLimits = io_viewspaceLimits;
}

//coord conversion-------------------------------------------------------------------------------------------------------

static void fromRowCol(int* ioRow, int* ioCol) {
    float helpRow = (float)*ioRow;
    float helpCol = (float)*ioCol;
    *ioRow = (helpRow / maxRows) * firstSize.y;
    *ioCol = (helpCol / maxCols) * firstSize.x;
}

static void fromRowColBounds(int* ioW, int* ioH) {
    auto size = Renderer::currentWindow->getSize();
    float helpW = (float)*ioW;
    float helpH = (float)*ioH;
    *ioW = (helpW / maxCols) * firstSize.x;
    *ioH = (helpH / maxRows) * firstSize.y;
}

static void toRowCol(int* io_X, int* io_Y) {
    auto size = Renderer::currentWindow->getSize();
    float helpRow = (float)*io_Y;
    float helpCol = (float)*io_X;
    *io_Y = (helpRow / size.y) * maxRows;
    *io_X = (helpCol / size.x) * maxCols;
}

//\coord conversion-------------------------------------------------------------------------------------------------------

//Drawing functions-------------------------------------------------------------------------------------------------------

void Renderer::drawRect(int row, int col, int width, int height, sf::Color c, bool solidWithViewspace) {
    fromRowColBounds(&width, &height);
    sf::RectangleShape* square = new sf::RectangleShape(sf::Vector2f(width, height));

    square->setFillColor(c);
    fromRowCol(&row, &col);
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
    int unusedHelp = 0;
    fromRowColBounds(&thickness, &unusedHelp);

    fromRowColBounds(&width, &height);
    sf::RectangleShape* square = new sf::RectangleShape(sf::Vector2f(width - thickness, height -  2 * thickness));
    square->setOutlineColor(c);
    square->setFillColor(sf::Color(0, 0, 0, 0));
    square->setOutlineThickness(thickness);


    fromRowCol(&row, &col);
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
    int unusedHelp = 0;
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
    fromRowCol(&row, &col);
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

    float dx = col2 - col1;
    float dy = row2 - row1;
    int ht = thickness / 2;
    float rot = atan2(dy, dx) * 57.2958f;
    sf::RectangleShape line = sf::RectangleShape(sf::Vector2f(std::sqrt(std::abs(dx) * std::abs(dx) + std::abs(dy) * std::abs(dy)), ht * 2));

    line.setOrigin(0, ht);
    line.setPosition(col1 - viewSpace [1], row1 - viewSpace [0]);
    line.setRotation(rot);
    line.setFillColor(c);

    Renderer::currentWindow->draw(line);
}

int mouseLimitRow, mouseLimitCol;
void Renderer::limitMouse(int row, int col) {
    mouseLimitRow = row;
    mouseLimitCol = col;
}

void Renderer::getMousePos(int* o_x, int* o_y, bool factorInViewspace, bool factorInBorders) {
    auto pos = sf::Mouse::getPosition();
    auto size = Renderer::currentWindow->getSize();
    auto offset = currentWindow->getPosition();
    int x = pos.x - offset.x;
    int y = pos.y - offset.y - 60;
    toRowCol(&x, &y);

    if (factorInBorders == true) {
        int limitRow = maxRows;
        int limitCol = maxCols;
        if (factorInViewspace == true) {
            limitRow = mouseLimitRow;
            limitCol = mouseLimitCol;
        }
        if (x < limitCol && y < limitRow) {
            if (factorInViewspace == true) {
                *o_x = x + viewSpace [1];
                *o_y = y + viewSpace [0];
            }
            else {
                *o_x = x;
                *o_y = y;
            }
        }
    }
    else {
        if (factorInViewspace == true) {
            *o_x = x + viewSpace [1];
            *o_y = y + viewSpace [0];
        }
        else {
            *o_x = x;
            *o_y = y;
        }
    }
}

void Renderer::updateViewSpace() {
    int moveSpeed = 30;
    
    int mouseX = -1;
    int mouseY = -1;
    getMousePos(&mouseX, &mouseY, false, true);
    int* helpViewSpace = new int[2];
    helpViewSpace[0] = viewSpace[0];
    helpViewSpace[1] = viewSpace[1];

    if (mouseX != -1) {
        int localRow = mouseY;
        int localCol = mouseX;

        if (localRow < maxRows / 10 && helpViewSpace[0] - moveSpeed > viewSpaceLimits[2]) {
            helpViewSpace[0] -= moveSpeed;
        }
        if (localRow > maxRows * 0.9 && helpViewSpace[0] + moveSpeed < viewSpaceLimits[3]) {
            helpViewSpace[0] += moveSpeed;
        }
        if (localCol < maxCols / 10 && helpViewSpace[1] - moveSpeed > viewSpaceLimits[0]) {
            helpViewSpace[1] -= moveSpeed;
        }
        if (localCol > maxCols * 0.9 && helpViewSpace[1] + moveSpeed < viewSpaceLimits[1]) {
            helpViewSpace[1] += moveSpeed;
        }
    }
    delete[] viewSpace;
    viewSpace = helpViewSpace;
}


sf::Texture Renderer::loadTexture(std::string path) {
    sf::Texture texture;
    if (!texture.loadFromFile(path))
    {
        std::cout << "failed to load texture of path '" << path << "'";
        std::exit(0);
    }
    return texture;
}


void Renderer::drawRectWithTexture(int row, int col, int width, int height, sf::Texture texture, bool solidWithViewspace) {
    fromRowColBounds(&width, &height);

    sf::RectangleShape* square = new sf::RectangleShape(sf::Vector2f(width, height));
    square->setTexture(&texture);
    fromRowCol(&row, &col);

    if (solidWithViewspace == true) {
        square->move(col, row);
    }
    else {
        square->move(col - viewSpace[1], row - viewSpace[0]);
    }

    Renderer::currentWindow->draw(*square);

    delete square;
}

void Renderer::drawText(std::string i_text, int row, int col, int width, int height, sf::Color color) {
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

    fromRowColBounds(&width, &height);
    fromRowCol(&row, &col);

    text.setCharacterSize(width / 4);//TODO window size reliants

    size_t CharacterSize = text.getCharacterSize();

    std::string String = text.getString().toAnsiString();
    bool bold = (text.getStyle() == sf::Text::Bold);
    size_t MaxHeight = 0;

    for (size_t x = 0; x < text.getString().getSize(); x++)
    {
        sf::Uint32 Character = String.at(x);

        const sf::Glyph& CurrentGlyph = font.getGlyph(Character, CharacterSize, bold);

        size_t Height = CurrentGlyph.bounds.height;

        if (MaxHeight < Height)
            MaxHeight = Height;
    }

    sf::FloatRect rect = text.getGlobalBounds();

    rect.left = ((float)width / 2.0f) - (rect.width / 2.0f);
    rect.top = ((float)height / 2.0f) - ((float)MaxHeight / 2.0f) - (rect.height - MaxHeight) + ((rect.height - CharacterSize) / 2.0f);

    int l = rect.left - 5;
    int t = rect.top - 15;
    fromRowCol(&l, &t);

    text.setPosition(l + col, t + row);

   // text.setStyle(sf::Text::Bold | sf::Text::Underlined);
    currentWindow->draw(text);
}