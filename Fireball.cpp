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

//create from network input(y is just current y so even with lag the start is always synced)
Fireball::Fireball(int i_currentY, int i_currentX, int i_goalY, int i_goalX, int i_myPlayerIndex,
    int i_phase, int i_timeSinceExplosionStart) : Ability(i_myPlayerIndex, true, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {

    this->startY = i_currentY;
    this->startX = i_currentX;
    this->goalY = i_goalY;
    this->goalX = i_goalX;
    this->tempTimeSinceExplosionStart = i_timeSinceExplosionStart;
    //start explosion, only useful if you have a big lag and the fireball gets transmitted only 
    //after exploding or you connect after explosion

    this->helpProjectile = new Projectile(startY, startX, velocity, goalY, goalX, false, radius,
        GlobalRecources::players[myPlayerIndex]);
    skipToPhase(i_phase);
}

void Fireball::execute0() {
    if (indicator->endWithoutAction() == true) {
        delete indicator;
        finished = true;
        indicator = nullptr;
    }
    else if (indicator->destinationSelected() == true) {
        goalY = indicator->getDestinationY();
        goalX = indicator->getDestinationX();
        delete indicator;
        indicator = nullptr;
        this->nextPhase();//init casting
    }
    else {
        indicator->update();
    }
}

void Fireball::init1() {
    Player* myPlayer = GlobalRecources::players[myPlayerIndex];
    //Turn player to mouse coords and set mouse coords as goal coords
    //if projectile destination is above player
    if (this->goalY < myPlayer->getY()) {
        this->startX = myPlayer->getX() + (myPlayer->getWidth() / 2);
        this->startY = myPlayer->getY();
        myPlayer->setTexture(2);
    }
    //below
    if (this->goalY > myPlayer->getY()) {
        this->startX = myPlayer->getX() + (myPlayer->getWidth() / 2);
        this->startY = myPlayer->getY() + myPlayer->getHeight();
        myPlayer->setTexture(3);
    }

    limitGoalPosToRange();
    this->helpProjectile = new Projectile(startY, startX, velocity, goalY, goalX, false, radius, myPlayer);
}

void Fireball::execute1() {
    auto colidables = GlobalRecources::terrain->getCollidables();
    this->helpProjectile->move(GlobalRecources::worldYs, GlobalRecources::worldXs, colidables->data(), colidables->size());
    //if the projectile reaches its max range or colides with anything, it should explode
    if ((abs(this->startY - this->helpProjectile->getY()) * abs(this->startY - this->helpProjectile->getY())
        + abs(this->startX - this->helpProjectile->getX()) * abs(this->startX - this->helpProjectile->getX())
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

    this->explosionY = this->helpProjectile->getY() + this->helpProjectile->getRadius() - this->explosionRange;
    this->explosionX = this->helpProjectile->getX() + this->helpProjectile->getRadius() - this->explosionRange;
}

void Fireball::execute2() {
    for (int i = 0; i < GlobalRecources::playerCount; i++) {
        Player* c = GlobalRecources::players[i];
        if (c->targetAble == true) {
            bool colision = Utils::colisionRectCircle(c->getX(), c->getY(), c->getWidth(), c->getHeight(),
                this->explosionX, this->explosionY, this->explosionRange, 10);
            if (colision == true) {
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
    Renderer::drawCircle(this->explosionX, this->explosionY, this->explosionRange, sf::Color(255, 120, 0, 255), true, 0, false);
}


void Fireball::limitGoalPosToRange() {
    float* vecToGoal = new float[2];
    vecToGoal[0] = goalX - startX;
    vecToGoal[1] = goalY - startY;
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
        goalX = startX + vecToGoal[0];
        goalY = startY + vecToGoal[1];
    }
}