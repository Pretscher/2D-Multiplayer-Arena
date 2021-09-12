#pragma once
#include "Ability.hpp"
#include "Projectile.hpp"
#include "Indicator.hpp"

class Transfusion : public Ability {
public:
    //normal constructor if the ability is created on your network-side
    Transfusion(int i_myPlayerIndex);
    //constructor through networking
    Transfusion(int i_myPlayerIndex, int i_targetPlayerIndex);

    //Phase 0: Indicator. Select target with point and click. If selected
    void execute0() override;
    void draw0() override;
    //Phase 1: find path to target player and follow him if neccessary. If in range, cast (phase 2) 
    void init1() override;
    void execute1() override;
    void draw1() override;
    //Phase 2: Cast. Bloodballs fire to enemy (follow him) and come back (follow you). When you are reached, finish ability
    void init2() override;
    void execute2() override;
    void draw2() override;

    //check if the bloodball colides with target player or you, depending if its flying to the enemy or you. 
    void checkBloodballXlision();
    //if player position changes, fly to new position. Needs some delay or else lag xD 
    //(Can fly through colidables so no pathfinding)
    void followPlayer();
    //get current bloodball target
    Player* getBloodballTarget();

    //get target player selected with indicator/passed through network
    inline int getTargetPlayer() {
        return targetPlayerIndex;
    }

private:
    Projectile* bloodBall = nullptr;//we check if it is a nullpr later
    int tempGoalY, tempGoalX;
    bool flyBack = false;//blood goes to target player and then flies back, we need a bool to determine this break point

    PointAndClickIndicator* indicator = nullptr;//we check if it is a nullpr later

    bool initializedEvents = false;

    int abilityPathIndex;//needed to determine wether another path has been found ->ability should be stopped

    Player* me;
    Player* target;
    //hardcoded stuff
    int dmg = 25;
    int heal = 25;
    int radius = 10;
    int velocity = 5.0f;
    int range = 300;

    int positionsSavedCount = 10;//amount of blood balls flowing around
    int* lastYs;
    int* lastXs;
    int cPositionSaveIndex = 0;

    int targetPlayerIndex;

    int findNewPathToPlayerTimer = 0;
};