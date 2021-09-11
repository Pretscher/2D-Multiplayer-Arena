#pragma once
#include "Ability.hpp"
#include "Indicator.hpp"
#include "Projectile.hpp"
class VladR : public Ability {
public:

    VladR(int i_myPlayerIndex);
    //create from network input(y is just current y so even with lag the start is always synced)
    VladR(int i_myPlayerIndex, int i_timeInPhase, int i_y, int i_x);

    //Has only one phase: being in pool
    void execute0() override;
    void draw0() override;

    void init1() override;
    void execute1() override;
    void draw1() override;

    void init2() override;
    void execute2() override;
    void draw2() override;

    void init3() override;
    void execute3() override;
    void draw3() override;

    inline int getY() {
        return y;
    }
    inline int getX() {
        return x;
    }

private:

    void checkBloodballXlision();
    void followPlayer();

    int damage = 50;
    int range = 300;
    int radius = 200;
    int timeTillProc = 4000;
    int healPerPlayer = 100;

    int y; 
    int x;
    AOEonRangeIndicator* indicator;

    int abilityPathIndex;//needed to determine wether another path has been found ->ability should be stopped

    Player** affectedPlayers;
    int affectedPlayerCount = 0;

    Projectile* bloodBall;
    int positionsSavedCount = 10;//amount of blood balls flowing around
    int* lastYs;
    int* lastXs;
    int cPositionSaveIndex = 0;
    float flyBackVelocity = 5.0f;
    int flyBackRadius = 10;

    int tempFlybackX;
    int tempFlybackY;

    int findNewPathToPlayerTimer = 0;

    int networkingTimeInPhase = 0;
};
