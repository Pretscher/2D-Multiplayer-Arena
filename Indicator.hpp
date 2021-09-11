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
        //go back to ary cursor if indicator is inactive (delete indicator)
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

                        int mouseX = 0, mouseY = 0;
                        Renderer::getMousePos(&mouseX, &mouseY, true, true);

                        if (Utils::xlisionCoordsRect(c->getX(), c->getY(), c->getWidth(),
                            c->getHeight(), mouseX, mouseY) == true) {
                            //IF LEFTCLICK HAS BEEN PRESSED (see above) select player
                            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                                this->targetIndex = i;
                            }
                            //draw an outline around the hovered over player
                            Renderer::drawRectOutline(c->getX(), c->getY(), c->getWidth(), c->getHeight(),
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
        int indicatorY = me->getY() + me->getHeight() / 2 - this->range;//range = radius of circle
        int indicatorX = me->getX() + me->getWidth() / 2 - this->range;

        Renderer::drawCircle(indicatorX, indicatorY, this->range, sf::Color(0, 255, 255, 100), false, 10, false);
        Renderer::drawCircle(indicatorX, indicatorY, this->range, sf::Color(0, 255, 255, 25), true, 0, false);
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
        //go back to ary cursor if indicator is inactive (delete indicator)
        sf::Cursor cursor;
        if (cursor.loadFromSystem(sf::Cursor::Arrow)) {
            Renderer::currentWindow->setMouseCursor(cursor);
        }
    }

    void update() {
        if (endWOaction == false) {
            Renderer::getMousePos(&cGoalX, &cGoalY, true, false);
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
        int playerCenterY = GlobalRecources::players[this->myPlayerIndex]->getY() + GlobalRecources::players[this->myPlayerIndex]->getHeight() / 2;
        int playerCenterX = GlobalRecources::players[this->myPlayerIndex]->getX() + GlobalRecources::players[this->myPlayerIndex]->getWidth() / 2;
        float* vecToGoal = new float[2];
        vecToGoal[0] = cGoalX - playerCenterX;
        vecToGoal[1] = cGoalY - playerCenterY;
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
            cGoalX = playerCenterX + vecToGoal[0];
            cGoalY = playerCenterY + vecToGoal[1];
        }
    }

    void draw() {
        int playerCenterY = GlobalRecources::players[this->myPlayerIndex]->getY() + GlobalRecources::players[this->myPlayerIndex]->getHeight() / 2;
        int playerCenterX = GlobalRecources::players[this->myPlayerIndex]->getX() + GlobalRecources::players[this->myPlayerIndex]->getWidth() / 2;
        //draw range indicator
        int indicatorY = playerCenterY - this->range;//range = radius of circle
        int indicatorX = playerCenterX - this->range;
        Renderer::drawCircle(indicatorX, indicatorY, this->range, sf::Color(0, 255, 255, 100), false, 10, false);


        Renderer::drawLine(playerCenterX, playerCenterY, cGoalY, cGoalX,
            sf::Color(0, 255, 255, 100), projectileRadius * 2);

        int innerCircleRadius = 50;
        Renderer::drawCircle(playerCenterX - innerCircleRadius, playerCenterY - innerCircleRadius,
                innerCircleRadius,sf::Color(0, 0, 0, 255), true, 0, false);
        Renderer::drawCircle(indicatorX, indicatorY, this->range, sf::Color(0, 255, 255, 25), true, 0, false);
    }

    inline bool endWithoutAction() {
        return endWOaction;
    }

    inline bool destinationSelected() {
        return fire;
    }

    inline int getDestinationY() {
        return cGoalY;
    }

    inline int getDestinationX() {
        return cGoalX;
    }

private:
    int cGoalY, cGoalX;

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
        //go back to ary cursor if indicator is inactive (delete indicator)
        sf::Cursor cursor;
        if (cursor.loadFromSystem(sf::Cursor::Arrow)) {
            Renderer::currentWindow->setMouseCursor(cursor);
        }
    }

    void update() {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            int mouseX = 0, mouseY = 0;
            Renderer::getMousePos(&mouseX, &mouseY, true, true);
            targetDestinationX = mouseX;
            targetDestinationY = mouseY;
            destSelected = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
            endWOaction = true;
        }
    }

    void draw() {
        int mouseY = 0;
        int mouseX = 0;
        Renderer::getMousePos(&mouseX, &mouseY, true, true);
        Renderer::drawCircle(mouseX - radius, mouseY - radius, radius, sf::Color(0, 255, 255, 100), false, 10, false);
        Renderer::drawCircle(mouseX - radius, mouseY - radius, radius, sf::Color(0, 255, 255, 25), true, 0, false);

        int playerCenterY = GlobalRecources::players[this->myPlayerIndex]->getY() + GlobalRecources::players[this->myPlayerIndex]->getHeight() / 2;
        int playerCenterX = GlobalRecources::players[this->myPlayerIndex]->getX() + GlobalRecources::players[this->myPlayerIndex]->getWidth() / 2;
        Renderer::drawCircle(playerCenterX - range, playerCenterY - range, range, sf::Color(0, 255, 255, 100), false, 10, false);
        Renderer::drawCircle(playerCenterX - range, playerCenterY - range, range, sf::Color(0, 255, 255, 25), true, 0, false);
    }

    inline bool isDestSelected() {
        return destSelected;
    }

    inline int getDestX() {
        return targetDestinationX;
    }
    inline int getDestY() {
        return targetDestinationY;
    }

    inline bool endWithoutAction() {
        return endWOaction;
    }

private:
    int radius;
    int range;
    int targetDestinationX = -1;
    int targetDestinationY = -1;
    int myPlayerIndex;

    bool destSelected = false;
    bool endWOaction = false;
};