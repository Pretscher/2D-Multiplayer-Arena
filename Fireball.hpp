#pragma once
#include "Ability.hpp"
#include "Indicator.hpp"
#include "Projectile.hpp"

class Fireball : public Ability {
public:
    Fireball(int i_myPlayerIndex);
    //create from network input(row is just current row so even with lag the start is always synced)
    Fireball(int i_currentRow, int i_currentCol, int i_goalRow, int i_goalCol, int i_myPlayerIndex,
                                                        int i_phase, int i_timeSinceExplosionStart);

    void execute0() override;
    void init1() override;
    void execute1() override;
    void init2() override;
    void execute2() override;
    void draw0() override;
    void draw1() override;
    void draw2() override;

    void limitGoalPosToRange();

public:

    inline int getStartRow() {
        return startRow;
    }
    inline int getStartCol() {
        return startCol;
    }
    inline int getGoalRow() {
        return goalRow;
    }
    inline int getGoalCol() {
        return goalCol;
    }
    inline int getProjectileRow() {
        return helpProjectile->getRow();
    }
    inline int getProjectileCol() {
        return helpProjectile->getCol();
    }
    inline bool hasFinishedNoCast() {
        return finishedNoCast;
    }

private:
    bool dealtDamage = false;
    int explosionRange = 80;
    int explosionRow, explosionCol;
    int explosionDuration;
    int explosionDmg;
    float burnDmg;

    int radius = 50;
    int range = 700;
    float velocity = 15.0f;

    Projectile* helpProjectile;
    ProjectileIndicator* indicator;

    //they have getters
    int startRow;
    int startCol;
    int goalRow;
    int goalCol;

    bool finishedNoCast = false;

    int tempTimeSinceExplosionStart = -1;
};