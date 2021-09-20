#include "Transfusion.hpp"
#include "GlobalRecources.hpp"


static int i_onCDPhase = 2;
static int i_addToNetworkPhase = 2;
static int i_abilityIndex = 0;


Transfusion::Transfusion(int i_myPlayerIndex) : Ability(i_myPlayerIndex, false, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {
    this->indicator = new PointAndClickIndicator(this->myPlayerIndex, this->range);

    lastYs = new int[positionsSavedCount];
    lastXs = new int[positionsSavedCount];
    for (int i = 0; i < positionsSavedCount; i++) {
        lastYs[i] = -1;
        lastXs[i] = -1;
    }
}
//constructor through networking
Transfusion::Transfusion() : Ability(NetworkCommunication::receiveNextToken(), true, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {
    this->targetPlayerIndex = NetworkCommunication::receiveNextToken();//we dont know the indicator so its target has to be passed
    
    lastYs = new int[positionsSavedCount];
    lastXs = new int[positionsSavedCount];
    for (int i = 0; i < positionsSavedCount; i++) {
        lastYs[i] = -1;
        lastXs[i] = -1;
    }

    //normally initialized in init1, but we have to skip this
    me = GlobalRecources::players[myPlayerIndex];
    target = GlobalRecources::players[targetPlayerIndex];

    skipToPhase(2);
}

void Transfusion::execute0() {
    if (indicator != nullptr && indicator->getTargetIndex() == -1) {
        indicator->update();
        if (indicator->endWithoutAction() == true) {
            finished = true;
            delete indicator;// we dont need this anymore
        }
    }
    else {
        targetPlayerIndex = indicator->getTargetIndex();
        delete indicator;// we dont need this anymore
        indicator = nullptr;//we check this in drawing caus bools are for noobs
        this->nextPhase();
    }
}

void Transfusion::draw0() {
    indicator->draw();
}

void Transfusion::init1() {
    me = GlobalRecources::players[myPlayerIndex];
    target = GlobalRecources::players[targetPlayerIndex];


    //if player out of range, run into range
    int halfW = me->getWidth() / 2;//we need this to calc the range between the player's coord centers
    int halfH = me->getHeight() / 2;
    float dist = Utils::calcDist2D(me->getX() + halfW, target->getX() + halfW,
        me->getY() + halfH, target->getY() + halfH);
    if (dist > range) {//if player is too far away
        tempGoalY = target->getY() + halfW;//find a path to his center because thats better than left top coords
        tempGoalX = target->getX() + halfH;
        GlobalRecources::pFinding->findPath(tempGoalX, tempGoalY, myPlayerIndex); //find a path to him
        abilityPathIndex = GlobalRecources::players[myPlayerIndex]->pathsFound;
    }
    else {
        nextPhase();//if already in range, just start casting without moving
    }
}

//find path to target player (damage part of succ)
void Transfusion::execute1() {
    int halfW = me->getWidth() / 2;
    if (tempGoalY != (target->getY() + halfW) || (tempGoalX != target->getX() + halfW)) {
        tempGoalY = target->getY() + halfW;
        tempGoalX = target->getX() + halfW;
        GlobalRecources::pFinding->findPath(tempGoalX, tempGoalY, myPlayerIndex);
        abilityPathIndex = GlobalRecources::players[myPlayerIndex]->pathsFound;
    }

    //multithreading problem: we want, if another path is found for the player (e.g. through clicking)
    //to stop going on the path the ability wants to find. But because of multithreading we cant say
    //when the pathfinding is finished with this particular path, so we just count the paths up in the
    //player obj and if the index is equal to abiltyPathIndex its still finding the path (same index
    //as when it was saved) and if its one higher the path was found. if its 2 higher a new path
    //was found and we interrupt.
    bool stop = false;
    if (GlobalRecources::players[myPlayerIndex]->pathsFound > abilityPathIndex + 1) {
        stop = true;
    }
    if (stop == false) {
        int halfW = me->getWidth() / 2;
        int halfH = me->getHeight() / 2;
        if (Utils::calcDist2D(me->getX() + halfW, target->getX() + halfW,
            me->getY() + halfH, target->getY() + halfH) < range) {
            //got into range, stop going on path an cast ability
            GlobalRecources::players[myPlayerIndex]->deletePath();
            nextPhase();
        }
    }
    else {
        //clicked somewhere else while finding path to target player to get in range => abort cast
        finished = true;
    }
}

void Transfusion::draw1() {

}

//init cast
void Transfusion::init2() {
    tempGoalY = target->getY();
    tempGoalX = target->getX();
    int halfW = me->getWidth() / 2;
    bloodBall = new Projectile(me->getY() + halfW, me->getX() + halfW, velocity,
        tempGoalY + halfW, tempGoalX + halfW, false, radius, me);

    for (int i = 0; i < positionsSavedCount; i++) {
        lastYs[i] = -1;
        lastXs[i] = -1;
    }
}

void Transfusion::execute2() {
    checkBloodballCollision();
    findNewPathToPlayerTimer ++;
    if (findNewPathToPlayerTimer % 10 == true) {
        followPlayer();
    }

    bloodBall->move(GlobalRecources::worldHeight, GlobalRecources::worldWidth, nullptr, 0);//should go through walls so we just dont pass them
}

void Transfusion::draw2() {
    lastYs[cPositionSaveIndex] = bloodBall->getY();
    lastXs[cPositionSaveIndex] = bloodBall->getX();
    cPositionSaveIndex ++;
    if (cPositionSaveIndex >= positionsSavedCount) {
        cPositionSaveIndex = 0;
    }
    bloodBall->draw(sf::Color(200, 0, 0, 255));
    for (int i = 0; i < positionsSavedCount; i++) {
        if (lastYs[i] != -1) {
            Renderer::drawCircle(lastXs[i], lastYs[i], bloodBall->getRadius(), sf::Color(200, 0, 0, 255), true, 0, false);
        }
    }
}

void Transfusion::checkBloodballCollision() {
    shared_ptr<Player> c = getBloodballTarget();
    //blood ball got to enemy and should fly back
    if (Utils::colisionRectCircle(c->getY(), c->getX(), c->getWidth(), c->getHeight(),
        bloodBall->getY(), bloodBall->getX(), bloodBall->getRadius(), 10) == true) {
        if (flyBack == false) {
            flyBack = true;
            if (target->targetAble == true) {
                target->setHp(target->getHp() - dmg);
            }
        }
        else {
            if (me->getHp() + heal <= me->getMaxHp()) {
                me->setHp(me->getHp() + heal);
            }
            else {
                me->setHp(me->getMaxHp());
            }
            finished = true;
        }
    }
}

void Transfusion::followPlayer() {
    shared_ptr<Player> c = getBloodballTarget();

    if (tempGoalY != c->getY() || tempGoalX != c->getX()) {
        tempGoalY = c->getY();
        tempGoalX = c->getX();
        int tempBBy = bloodBall->getY();
        int tempBBx = bloodBall->getX();


        int halfW = me->getWidth() / 2;
        delete bloodBall;//definitly exists at this point so we can delete it
        bloodBall = new Projectile(tempBBy + radius, tempBBx + radius, velocity,
            tempGoalY + halfW, tempGoalX + halfW, false, radius, me);
    }
}

shared_ptr<Player> Transfusion::getBloodballTarget() {
    shared_ptr<Player> c;
    if (flyBack == false) {
        c = target;
    }
    else {
        c = me;
    }
    return c;
}

void Transfusion::send() {
    NetworkCommunication::addToken(this->myPlayerIndex);
    NetworkCommunication::addToken(this->targetPlayerIndex);
}