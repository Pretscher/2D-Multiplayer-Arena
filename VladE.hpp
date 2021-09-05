#pragma once
#include "Ability.hpp"
#include "Projectile.hpp"

class VladE : public Ability {
public:
   
    VladE(int i_myPlayerIndex);
    //create from network input(row is just current row so even with lag the start is always synced)
    VladE(int i_currentRow, int i_currentCol, int i_phase, int i_timeInPhase);

    //phase 0: Indicator. Select destination for fireball
    void execute0() override;
    void draw0() override;
    //phase 2: Fireball flies
    void init1() override;//cast fireball
    void execute1() override;//fireball flies to destination/player/terrain and initiates explosion there
    void draw1() override;
    //phase 3: Explosion. Fire on ground and initial aoe damage. After time interval, explosion goes away. Then finish.
    void init2() override;
    void execute2() override;
    void draw2() override;

private:
    int radius = 50;

    int range = 300;
    float velocity = 15.0f;

    Projectile** projectiles;
    int projectileCount;

    bool finishedNoCast = false;
    float percentFinishedCharge;
};