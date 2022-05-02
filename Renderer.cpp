#include "Renderer.hpp"
#include <iostream>
#include <filesystem>
#include <math.h>
#include "GlobalRecources.hpp"
//Call this-----------------------------------------------------------------------------------------------------------

shared_ptr<int[]> Renderer::viewSpace;
shared_ptr<const int[]>  Renderer::viewSpaceLimits;
shared_ptr<sf::RenderWindow> Renderer::currentWindow;

void Renderer::init(shared_ptr<sf::RenderWindow> window) {
    currentWindow = window;
    currentWindow->setPosition(sf::Vector2i(-13, -13));//fsr thats left top, good library
}

void Renderer::linkViewSpace(shared_ptr<int[]> io_viewSpace, shared_ptr<const int[]> io_viewspaceLimits) {
    viewSpace = io_viewSpace;
    viewSpaceLimits = io_viewspaceLimits;

    GlobalRecources::worldHeight = getWorldYs();//Set this here caus mouse pos is always called and im too lazy to make a rendered update func
    GlobalRecources::worldWidth = getWorldXs();
}

int Renderer::getWorldXs() {
    return currentWindow->getSize().x + viewSpaceLimits[1];
}

int Renderer::getWorldYs() {
    return currentWindow->getSize().y + viewSpaceLimits[0];
}

//coord conversion-------------------------------------------------------------------------------------------------------

int normalResXs = 1920;
int normalResYs = 1080;

static void fromYX(int& ioY, int& ioX) {
    auto size = Renderer::currentWindow->getSize();
    ioY = ((float) ioY / normalResYs) * size.y;
    ioX = ((float) ioX / normalResXs) * size.x;
}

static void fromYXBounds(int& ioW, int& ioH) {
    auto size = Renderer::currentWindow->getSize();
    ioW = ((float) ioW / normalResXs) * size.x;
    ioH = ((float) ioH / normalResYs) * size.y;
}

static void toYX(int& ioY, int& ioX) {
    auto size = Renderer::currentWindow->getSize();
    ioY = ((float)ioY / size.y) * normalResYs;
    ioX = ((float)ioX / size.x) * normalResXs;
}

static void toYXBounds(int& ioW, int& ioH) {
    auto size = Renderer::currentWindow->getSize();
    ioW = ((float)ioW / normalResXs) * size.x;
    ioH = ((float)ioH / normalResYs) * size.y;
}
//\coord conversion-------------------------------------------------------------------------------------------------------


//Drawing functions-------------------------------------------------------------------------------------------------------

void Renderer::drawRect(int x, int y, int width, int height, sf::Color c, bool solidWithViewspace) {
    fromYXBounds(width, height);
    fromYX(y, x);

    sf::RectangleShape square(sf::Vector2f(width, height));

    square.setFillColor(c);
    if (solidWithViewspace == true) {
        square.move(x, y);
    }
    else {
        square.move(x - viewSpace[1], y - viewSpace[0]);
    }
    Renderer::currentWindow->draw(square);
}


void Renderer::drawRectOutline(int x, int y, int width, int height, sf::Color c, int thickness, bool solidWithViewspace) {
    fromYXBounds(width, height);
    fromYX(y, x);
    int unusedHelp = 0;//we dont need to write 2 values but only have functions for 2 values (lazyness)
    fromYXBounds(thickness, unusedHelp);

    sf::RectangleShape square(sf::Vector2f(width - thickness, height -  2 * thickness));
    square.setOutlineColor(c);
    square.setFillColor(sf::Color(0, 0, 0, 0));
    square.setOutlineThickness(thickness);

    if (solidWithViewspace == true) {
        square.move(x, y);
    }
    else {
        square.move(x - viewSpace[1], y - viewSpace[0]);
    }
    Renderer::currentWindow->draw(square);
}

void Renderer::drawCircle(int x, int y, int radius, sf::Color c, bool fill, int outlineThickness, bool solidWithViewspace) {
    fromYX(y, x);
    int unusedHelp = 0;//we dont need to write 2 values but only have functions for 2 values (lazyness)
    fromYXBounds(outlineThickness, unusedHelp);
    fromYXBounds(radius, unusedHelp);

    sf::CircleShape circle(radius);
    if (fill == true) {
        circle.setFillColor(c);
    }
    else {
        circle.setOutlineColor(c);
        circle.setOutlineThickness(outlineThickness);
        circle.setFillColor(sf::Color(0, 0, 0, 0));
    }
    if (solidWithViewspace == true) {
        circle.move(x, y);
    }
    else {
        circle.move(x - viewSpace[1], y - viewSpace[0]);
    }
    Renderer::currentWindow->draw(circle);
}

void Renderer::drawLine(int x1, int y1, int x2, int y2, sf::Color c, int thickness) {
    fromYX(y1, x1);
    fromYX(y2, x2);

    float dX = x2 - x1;
    float dY = y2 - y1;
    int ht = thickness / 2;
    float rot = atan2(dY, dX) * 57.2958f;
    sf::RectangleShape line = sf::RectangleShape(sf::Vector2f(sqrt(abs(dX) * abs(dX) + abs(dY) * abs(dY)), ht * 2));

    line.setOrigin(0, ht);
    line.setPosition(x1 - viewSpace[1], y1 - viewSpace[0]);
    line.setRotation(rot);
    line.setFillColor(c);

    Renderer::currentWindow->draw(line);
}

void Renderer::getMousePos(int& o_xs, int& o_ys, bool factorInViewspace, bool factorInBorders) {
    GlobalRecources::worldHeight = getWorldYs();//Set this here caus mouse pos is always called and im too lazy to make a rendered update func
    GlobalRecources::worldWidth = getWorldXs();

    auto pos = sf::Mouse::getPosition(*currentWindow);
    int x = pos.x;
    int y = pos.y;
    toYX(y, x);

    int helpVsX = viewSpace[1];
    int helpVsY = viewSpace[0];
    toYX(helpVsX, helpVsY);

    if (factorInBorders == true) {
        if (x < normalResXs && y < normalResYs) {
            if (factorInViewspace == true) {
                o_xs = x + helpVsX;
                o_ys = y + helpVsY;
            }
            else {
                o_xs = x;
                o_ys = y;
            }
        }
    }
    else {
        if (factorInViewspace == true) {
            o_xs = x + helpVsX;
            o_ys = y + helpVsY;
        }
        else {
            o_xs = x;
            o_ys = y;
        }
    }
}

void Renderer::updateViewSpace() {
    auto size = currentWindow->getSize();

    int moveSpeed = 30;
    
    int mouseX = -1, mouseY = -1;
     getMousePos(mouseX, mouseY, false, true);
    int helpViewSpace[2];//temp viewspace, maybe going to be written into real viewspace
    helpViewSpace[0] = viewSpace[0];
    helpViewSpace[1] = viewSpace[1];


    int vsLeft = viewSpaceLimits[0];
    int vsRight = viewSpaceLimits[1];
    int vsTop = viewSpaceLimits[2];
    int vsBottom = viewSpaceLimits[3];
    toYXBounds(vsTop, vsLeft);
    toYXBounds(vsBottom, vsRight);
    if (mouseY != -1) {
        int localY = mouseY;
        int localX = mouseX;
        fromYX(mouseY, mouseX);

        if (localY < normalResYs / 10) {
            if (helpViewSpace[0] - moveSpeed > vsTop) {
                helpViewSpace[0] -= moveSpeed;
            }
            else {
                helpViewSpace[0] = vsTop;
            }
        }
        if (localY > normalResYs * 0.9) {
            if (helpViewSpace[0] + moveSpeed < vsBottom) {
                helpViewSpace[0] += moveSpeed;
            }
            else {
                helpViewSpace[0] = vsBottom;
            }
        }
        if (localX < normalResXs / 10) {
            if (helpViewSpace[1] - moveSpeed > vsLeft) {
                helpViewSpace[1] -= moveSpeed;
            }
            else {
                helpViewSpace[1] = vsLeft;
            }
        }
        if (localX > normalResXs * 0.9f) {
            if (helpViewSpace[1] + moveSpeed < vsRight) {
                helpViewSpace[1] += moveSpeed;
            }
            else {
                helpViewSpace[1] = vsRight;
            }
        }
    }
    viewSpace[0] = helpViewSpace[0];
    viewSpace[1] = helpViewSpace[1];
}

sf::Texture Renderer::loadTexture(string path, bool repeat) {
    sf::Texture texture;
    if (!texture.loadFromFile(path))
    {
        cout << "failed to load texture of path '" << path << "'";
        exit(0);
    }
    if (repeat == true) {
        sf::Image img = texture.copyToImage();
        if (texture.loadFromImage(img) == false) {
            cout << "failed to load texture image of path '" << path << "'";
            exit(0);
        }

        texture.setRepeated(true);
    }
    return texture;
}


void Renderer::drawRectWithTexture(int x, int y, int width, int height, sf::Texture texture, bool solidWithViewspace) {
    fromYXBounds(width, height);
    fromYX(y, x);

    sf::RectangleShape square(sf::Vector2f(width, height));
    
    square.setTexture(&texture);
    if (solidWithViewspace == true) {
        square.move(x, y);
    }
    else {
        square.move(x - viewSpace[1], y - viewSpace[0]);
    }

    if (texture.isRepeated() == true) {
        sf::IntRect rect = sf::IntRect(x, y, width, height);
        square.setTextureRect(rect);
    }//else stretch

    Renderer::currentWindow->draw(square);
}

void Renderer::drawText(string i_text, int x, int y, int width, int height, int charSize, sf::Color color) {
    fromYXBounds(width, height);
    fromYX(y, x);

    sf::Text text;

    sf::Font font;
    if (!font.loadFromFile("Calibri Regular.ttf"))
    {
        cout << "error loading font.";
    }

    // select the font
    text.setFont(font); // font is a sf::Font
    text.setString(i_text);


    int unusedHelp = 0;
    toYXBounds(charSize, unusedHelp);
    text.setFillColor(color);
    text.setCharacterSize(charSize);

    size_t CharacterSize = text.getCharacterSize();


    string String = text.getString().toAnsiString();
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

    int l = rect.left - (currentWindow->getSize().x / 385);
    int t = rect.top - (currentWindow->getSize().y / 72);

    text.setPosition(l + x, t + y);

   // text.setStyle(sf::Text::Bold | sf::Text::Underlined);
    currentWindow->draw(text);
}