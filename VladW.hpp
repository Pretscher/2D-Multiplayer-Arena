#pragma once
#include "Ability.hpp"
#include <memory>

class VladW : public Ability {
public:

    VladW(int i_myPlayerIndex);
    //create from network input(y is just current y so even with lag the start is always synced)
    VladW();

    //Has only one phase: being in pool
    void execute0() override;
    void draw0() override;
    void init1() override;

    void send() override;
private:
    int damage = 1;
    int range = 150;
};