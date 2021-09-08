#pragma once
#include "Ability.hpp"

class VladW : public Ability {
public:

    VladW(int i_myPlayerIndex);
    //create from network input(row is just current row so even with lag the start is always synced)
    VladW(int i_myPlayerIndex, int i_timeInPhase);

    //Has only one phase: being in pool
    void execute0() override;
    void draw0() override;
    void init1() override;

private:
    int damage = 1;
    int range = 150;
};