#pragma once
#include "Ability.hpp"
#include "Indicator.hpp"
#include "Projectile.hpp"

class Fireball : public Ability {
public:
    //normal constructor if the ability is created on your network-side
    Fireball(int i_myPlayerIndex);
    //create from network input(y is just current y so even with lag the start is always synced)
    Fireball();

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

    void send() override;
    //If goal pos (indicator click) is outside of range, set goal pos to nearest point on range circle (through vector math)
    void limitGoalPosToRange();

public:
    inline int getStartY() {
        return startY;
    }
    inline int getStartX() {
        return startX;
    }
    inline int getGoalY() {
        return goalY;
    }
    inline int getGoalX() {
        return goalX;
    }
    inline int getProjectileY() {
        return helpProjectile->getY();
    }
    inline int getProjectileX() {
        return helpProjectile->getX();
    }

private:
    bool dealtDamage = false;
    int explosionRange = 80;
    int explosionY, explosionX;
    int explosionDuration;
    int explosionDmg;
    float burnDmg;

    int radius = 50;
    int range = 700;
    float velocity = 15.0f;

    Projectile* helpProjectile;
    ProjectileIndicator* indicator;

    //they have getters
    int startY;
    int startX;
    int goalY;
    int goalX;

    int tempTimeSinceExplosionStart = -1;
};