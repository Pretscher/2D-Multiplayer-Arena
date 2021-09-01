#pragma once
#include "Ability.hpp"
#include "Projectile.hpp"
#include "Indicator.hpp"

class Transfusion : public Ability {
public:
    Transfusion(int i_myPlayerIndex);
    //constructor through networking
    Transfusion(int i_myPlayerIndex, int i_targetPlayerIndex);
    void execute0() override;
    void draw0() override;
    void init1() override;
    //find path to target player (damage part of succ)
    void execute1() override;
    void draw1() override;
    //init cast
    void init2() override;
    void execute2() override;
    void draw2() override;

    void checkBloodballCollision();
    void followPlayer();
    Player* getBloodballTarget();

    //getters

    inline int getTargetPlayer() {
        return targetPlayerIndex;
    }

private:
    Projectile* bloodBall = nullptr;//we check if it is a nullpr later
    int tempGoalRow, tempGoalCol;
    bool flyBack = false;//blood goes to target player and then flies back, we need a bool to determine this break point

    PointAndClickIndicator* indicator = nullptr;//we check if it is a nullpr later

    bool initializedEvents = false;

    int abilityPathIndex;

    Player* me;
    Player* target;
    //hardcoded stuff
    int dmg = 25;
    int heal = 25;
    int radius = 10;
    int velocity = 5.0f;
    int range = 300;

    int positionsSavedCount = 10;//amount of blood balls flowing around
    int* lastRows;
    int* lastCols;
    int cPositionSaveIndex = 0;

    int targetPlayerIndex;

    int findNewPathToPlayerTimer = 0;
};