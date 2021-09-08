#pragma once
#include "Renderer.hpp"
#include "Player.hpp"
#include "Utils.hpp"
#include <math.h>
#include "GlobalRecources.hpp"
//Point and click indicator class-------------------------------------------------------------------

class PointAndClickIndicator {
public:
    PointAndClickIndicator(int i_castingPlayer, int i_range) {
        this->myPlayerIndex = i_castingPlayer;
        this->range = i_range;
        //set cursor to cross while indicator is active
        sf::Cursor cursor;
        if (cursor.loadFromSystem(sf::Cursor::Cross)) {
            Renderer::currentWindow->setMouseCursor(cursor);
        }
    }

    ~PointAndClickIndicator() {
        //go back to arrow cursor if indicator is inactive (delete indicator)
        sf::Cursor cursor;
        if (cursor.loadFromSystem(sf::Cursor::Arrow)) {
            Renderer::currentWindow->setMouseCursor(cursor);
        }
    }
    void update() {
        //if true, end next tick (if no player was selected just do nothing and end indicator)
        for (int i = 0; i < GlobalRecources::playerCount; i++) {
            if (i != this->myPlayerIndex) {
                Player* c = GlobalRecources::players[i];
                if(c->targetAble == true) {
                    if (c->getHp() > 0) {
                        int mouseRow = 0;
                        int mouseCol = 0;
                        Renderer::getMousePos(&mouseCol, &mouseRow, true, true);
                        if (Utils::collisionCoordsRect(c->getCol(), c->getRow(), c->getWidth(),
                            c->getHeight(), mouseCol, mouseRow) == true) {
                            //IF LEFTCLICK HAS BEEN PRESSED (see above) select player
                            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                                this->targetIndex = i;
                            }
                            //draw an outline around the hovered over player
                            Renderer::drawRectOutline(c->getRow(), c->getCol(), c->getWidth(), c->getHeight(),
                                sf::Color(75, 165, 180, 150), 2, false);
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
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
            endWOaction = true;
        }
    }

    void draw() {
        Player* me = GlobalRecources::players[this->myPlayerIndex];
        //draw range indicator
        int indicatorRow = me->getRow() + me->getHeight() / 2 - this->range;//range = radius of circle
        int indicatorCol = me->getCol() + me->getWidth() / 2 - this->range;

        Renderer::drawCircle(indicatorRow, indicatorCol, this->range, sf::Color(0, 255, 255, 100), false, 10, false);
        Renderer::drawCircle(indicatorRow, indicatorCol, this->range, sf::Color(0, 255, 255, 25), true, 0, false);
    }

    inline int getTargetIndex() {
        return this->targetIndex;
    }

    inline bool endWithoutAction() {
        return endWOaction;
    }

private:
    int range;
    int targetIndex = -1;
    int myPlayerIndex;

    bool endWOaction = false;
};

class ProjectileIndicator {
public:
    ProjectileIndicator(int i_castingPlayer, int i_range, int i_projRadius) {
        this->myPlayerIndex = i_castingPlayer;
        this->range = i_range;
        this->projectileRadius = i_projRadius;

        //set cursor to cross while indicator is active
        sf::Cursor cursor;
        if (cursor.loadFromSystem(sf::Cursor::Cross)) {
            Renderer::currentWindow->setMouseCursor(cursor);
        }
    }

    ~ProjectileIndicator() {
        //go back to arrow cursor if indicator is inactive (delete indicator)
        sf::Cursor cursor;
        if (cursor.loadFromSystem(sf::Cursor::Arrow)) {
            Renderer::currentWindow->setMouseCursor(cursor);
        }
    }

    void update() {
        if (endWOaction == false) {
            Renderer::getMousePos(&cGoalCol, &cGoalRow, true, false);
            limitGoalPosToRange();
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                fire = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
                endWOaction = true;
            }
        }
    }

    void limitGoalPosToRange() {
        int playerCenterRow = GlobalRecources::players[this->myPlayerIndex]->getRow() + GlobalRecources::players[this->myPlayerIndex]->getHeight() / 2;
        int playerCenterCol = GlobalRecources::players[this->myPlayerIndex]->getCol() + GlobalRecources::players[this->myPlayerIndex]->getWidth() / 2;
        float* vecToGoal = new float[2];
        vecToGoal[0] = cGoalCol - playerCenterCol;
        vecToGoal[1] = cGoalRow - playerCenterRow;
        //calculate vector lenght
        float lenght = sqrt((vecToGoal[0] * vecToGoal[0]) + (vecToGoal[1] * vecToGoal[1]));
        if (lenght > range) {
            //normalize vector lenght
            vecToGoal[0] /= lenght;
            vecToGoal[1] /= lenght;
            //stretch vector to range
            vecToGoal[0] *= range;
            vecToGoal[1] *= range;
            //place at starting point
            cGoalCol = playerCenterCol + vecToGoal[0];
            cGoalRow = playerCenterRow + vecToGoal[1];
        }
    }

    void draw() {
        int playerCenterRow = GlobalRecources::players[this->myPlayerIndex]->getRow() + GlobalRecources::players[this->myPlayerIndex]->getHeight() / 2;
        int playerCenterCol = GlobalRecources::players[this->myPlayerIndex]->getCol() + GlobalRecources::players[this->myPlayerIndex]->getWidth() / 2;
        //draw range indicator
        int indicatorRow = playerCenterRow - this->range;//range = radius of circle
        int indicatorCol = playerCenterCol - this->range;
        Renderer::drawCircle(indicatorRow, indicatorCol, this->range, sf::Color(0, 255, 255, 100), false, 10, false);


        Renderer::drawLine(playerCenterRow, playerCenterCol, cGoalRow, cGoalCol,
            sf::Color(0, 255, 255, 100), projectileRadius * 2);

        int innerCircleRadius = 50;
        Renderer::drawCircle(playerCenterRow - innerCircleRadius, playerCenterCol - innerCircleRadius,
                innerCircleRadius,sf::Color(0, 0, 0, 255), true, 0, false);
        Renderer::drawCircle(indicatorRow, indicatorCol, this->range, sf::Color(0, 255, 255, 25), true, 0, false);
    }

    inline bool endWithoutAction() {
        return endWOaction;
    }

    inline bool destinationSelected() {
        return fire;
    }

    inline int getDestinationRow() {
        return cGoalRow;
    }

    inline int getDestinationCol() {
        return cGoalCol;
    }

private:
    int cGoalRow, cGoalCol;

    int range;
    int projectileRadius;

    bool endWOaction = false;
    bool fire = false;

    int myPlayerIndex;
};

class AOEonRangeIndicator {
public:
    AOEonRangeIndicator(int i_castingPlayer, int i_range, int i_radius) {
        this->myPlayerIndex = i_castingPlayer;
        this->range = i_range;
        this->radius = i_radius;

        //set cursor to cross while indicator is active
        sf::Cursor cursor;
        if (cursor.loadFromSystem(sf::Cursor::Cross)) {
            Renderer::currentWindow->setMouseCursor(cursor);
        }
    }

    ~AOEonRangeIndicator() {
        //go back to arrow cursor if indicator is inactive (delete indicator)
        sf::Cursor cursor;
        if (cursor.loadFromSystem(sf::Cursor::Arrow)) {
            Renderer::currentWindow->setMouseCursor(cursor);
        }
    }

    void update() {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            int mouseRow = 0;
            int mouseCol = 0;
            Renderer::getMousePos(&mouseCol, &mouseRow, true, true);
            targetDestinationCol = mouseCol;
            targetDestinationRow = mouseRow;
            destSelected = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
            endWOaction = true;
        }
    }

    void draw() {
        int mouseRow = 0;
        int mouseCol = 0;
        Renderer::getMousePos(&mouseCol, &mouseRow, true, true);
        Renderer::drawCircle(mouseRow - radius, mouseCol - radius, radius, sf::Color(0, 255, 255, 100), false, 10, false);
        Renderer::drawCircle(mouseRow - radius, mouseCol - radius, radius, sf::Color(0, 255, 255, 25), true, 0, false);

        int playerCenterRow = GlobalRecources::players[this->myPlayerIndex]->getRow() + GlobalRecources::players[this->myPlayerIndex]->getHeight() / 2;
        int playerCenterCol = GlobalRecources::players[this->myPlayerIndex]->getCol() + GlobalRecources::players[this->myPlayerIndex]->getWidth() / 2;
        Renderer::drawCircle(playerCenterRow - range, playerCenterCol - range, range, sf::Color(0, 255, 255, 100), false, 10, false);
        Renderer::drawCircle(playerCenterRow - range, playerCenterCol - range, range, sf::Color(0, 255, 255, 25), true, 0, false);
    }

    inline bool isDestSelected() {
        return destSelected;
    }

    inline int getDestCol() {
        return targetDestinationCol;
    }
    inline int getDestRow() {
        return targetDestinationRow;
    }

    inline bool endWithoutAction() {
        return endWOaction;
    }

private:
    int radius;
    int range;
    int targetDestinationCol = -1;
    int targetDestinationRow = -1;
    int myPlayerIndex;

    bool destSelected = false;
    bool endWOaction = false;
};