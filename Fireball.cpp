#include "Fireball.hpp"
#include "GlobalRecources.hpp"
#include <chrono>
using namespace std::chrono;

static int i_onCDPhase = 1;
static int i_addToNetworkPhase = 2;
static int i_abilityIndex = 4;

Fireball::Fireball(int i_myPlayerIndex) : Ability(i_myPlayerIndex, false, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {//both constructors are used
    indicator = new ProjectileIndicator(i_myPlayerIndex, this->range, this->radius);
}

//create from network input(row is just current row so even with lag the start is always synced)
Fireball::Fireball(int i_currentRow, int i_currentCol, int i_goalRow, int i_goalCol, int i_myPlayerIndex,
    int i_phase, int i_timeSinceExplosionStart) : Ability(i_myPlayerIndex, true, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {

    this->startRow = i_currentRow;
    this->startCol = i_currentCol;
    this->goalRow = i_goalRow;
    this->goalCol = i_goalCol;
    this->tempTimeSinceExplosionStart = i_timeSinceExplosionStart;
    //start explosion, only useful if you have a big lag and the fireball gets transmitted only 
    //after exploding or you connect after explosion

    this->helpProjectile = new Projectile(startRow, startCol, velocity, goalRow, goalCol, false, radius,
        GlobalRecources::players[myPlayerIndex]);
    skipToPhase(i_phase);
}

void Fireball::execute0() {
    if (finishedNoCast == false) {
        if (indicator->endWithoutAction() == true) {
            delete indicator;
            finishedNoCast = true;
            indicator = nullptr;
        }
        else if (indicator->destinationSelected() == true) {
            goalRow = indicator->getDestinationRow();
            goalCol = indicator->getDestinationCol();
            delete indicator;
            indicator = nullptr;
            this->nextPhase();//init casting
        }
        else {
            indicator->update();
        }
    }
}

void Fireball::init1() {
    Player* myPlayer = GlobalRecources::players[myPlayerIndex];
    //Turn player to mouse coords and set mouse coords as goal coords
    //if projectile destination is above player
    if (this->goalRow < myPlayer->getRow()) {
        this->startCol = myPlayer->getCol() + (myPlayer->getWidth() / 2);
        this->startRow = myPlayer->getRow();
        myPlayer->setTexture(2);
    }
    //below
    if (this->goalRow > myPlayer->getRow()) {
        this->startCol = myPlayer->getCol() + (myPlayer->getWidth() / 2);
        this->startRow = myPlayer->getRow() + myPlayer->getHeight();
        myPlayer->setTexture(3);
    }

    limitGoalPosToRange();
    this->helpProjectile = new Projectile(startRow, startCol, velocity, goalRow, goalCol, false, radius, myPlayer);
}

void Fireball::execute1() {
    auto collidables = GlobalRecources::terrain->getCollidables();
    this->helpProjectile->move(GlobalRecources::worldRows, GlobalRecources::worldCols, collidables->data(), collidables->size());
    //if the projectile reaches its max range or collides with anything, it should explode
    if ((abs(this->startRow - this->helpProjectile->getRow()) * abs(this->startRow - this->helpProjectile->getRow())
        + abs(this->startCol - this->helpProjectile->getCol()) * abs(this->startCol - this->helpProjectile->getCol())
                > this->range * this->range) || this->helpProjectile->isDead() == true) {

        this->nextPhase(); //init explosion
    }
}

void Fireball::init2() {
    explosionDmg = 30;
    burnDmg = 0.25f;
    explosionDuration = 2000;
    dealtDamage = false;
    explosionRange = 80;

    if (tempTimeSinceExplosionStart < 0) {//if sent after explosion
        this->endPhaseAfterMS(explosionDuration);
    }
    else {
        auto cTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        int duration = explosionDuration - (cTime - tempTimeSinceExplosionStart);
        this->endPhaseAfterMS(duration);

    }

    this->explosionRow = this->helpProjectile->getRow() + this->helpProjectile->getRadius() - this->explosionRange;
    this->explosionCol = this->helpProjectile->getCol() + this->helpProjectile->getRadius() - this->explosionRange;
}

void Fireball::execute2() {
    for (int i = 0; i < GlobalRecources::playerCount; i++) {
        Player* c = GlobalRecources::players[i];
        if (c->targetAble == true) {
            bool collision = Utils::collisionRectCircle(c->getCol(), c->getRow(), c->getWidth(), c->getHeight(),
                this->explosionCol, this->explosionRow, this->explosionRange, 10);
            if (collision == true) {
                if (this->dealtDamage == false) {
                    c->setHp(c->getHp() - this->explosionDmg);
                }
                else {
                    c->setHp(c->getHp() - this->burnDmg);
                }
            }
        }
    }
    this->dealtDamage = true;
}

void Fireball::draw0() {
    if (indicator != nullptr) {
        indicator->draw();
    }
}

void Fireball::draw1() {
    this->helpProjectile->draw(sf::Color(255, 120, 0, 255));
}

void Fireball::draw2() {
    Renderer::drawCircle(this->explosionRow, this->explosionCol, this->explosionRange, sf::Color(255, 120, 0, 255), true, 0, false);
}


void Fireball::limitGoalPosToRange() {
    float* vecToGoal = new float[2];
    vecToGoal[0] = goalCol - startCol;
    vecToGoal[1] = goalRow - startRow;
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
        goalCol = startCol + vecToGoal[0];
        goalRow = startRow + vecToGoal[1];
    }
}