#include "Renderer.hpp"
#include "Player.hpp"
#include "Utils.hpp"

//Point and click indicator class-------------------------------------------------------------------

class PointAndClickIndicator {
public:
    PointAndClickIndicator(int i_playerIndex, int i_range, int i_playerCount, Player** i_players, int i_castingPlayer) {
        this->myPlayerIndex = i_playerIndex;
        this->range = i_range;
        this->playerCount = i_playerCount;
        this->players = i_players;
        this->myPlayerIndex = i_castingPlayer;
    }

    void update() {
        sf::Cursor cursor;
        if (cursor.loadFromSystem(sf::Cursor::Cross)) {
            Renderer::currentWindow->setMouseCursor(cursor);
        }

        //if true, end next tick (if no player was selected just do nothing and end indicator)

        for (int i = 0; i < playerCount; i++) {
            if (i != this->myPlayerIndex) {
                Player* c = players [i];
                if (c->getHp() > 0) {
                    int mouseRow = 0;
                    int mouseCol = 0;
                    Renderer::getMousePos(&mouseCol, &mouseRow, true);
                    if (Utils::collisionCoordsRect(c->getCol(), c->getRow(), c->getWidth(),
                        c->getHeight(), mouseCol, mouseRow) == true) {
                        //IF LEFTCLICK HAS BEEN PRESSED (see above) select player
                        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                            this->targetIndex = i;
                        }
                        //draw an outline around the hovered over player
                        Renderer::drawRectOutline(c->getRow(), c->getCol(), c->getWidth(), c->getHeight(),
                            sf::Color(75, 165, 180, 150), 5, false);
                    }
                    else {
                        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                            endWOaction = true;
                        }
                    }
                }
            }
        }
    }

    void draw() {
        //draw range indicator
        int indicatorRow = players [this->myPlayerIndex]->getRow() +
            players [this->myPlayerIndex]->getHeight() / 2 - this->range;//range = radius of circle
        int indicatorCol = players [this->myPlayerIndex]->getCol() +
            players [this->myPlayerIndex]->getWidth() / 2 - this->range;
        Renderer::drawCircle(indicatorRow, indicatorCol, this->range, sf::Color(0, 255, 255, 100), false, 10, false);
        Renderer::drawCircle(indicatorRow, indicatorCol, this->range, sf::Color(0, 255, 255, 25), true, 0, false);
    }

    int getTargetIndex() {
        return this->targetIndex;
    }

    bool endWithoutAction() {
        return endWOaction;
    }
private:
    int range;
    bool endWOaction = false;

    int playerCount;
    Player** players;
    int myPlayerIndex;
    int targetIndex = -1;
};
