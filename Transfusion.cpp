#include "Transfusion.hpp"
#include "GlobalRecources.hpp"


static int i_onCDPhase = 2;
static int i_addToNetworkPhase = 2;
static int i_abilityIndex = 1;


Transfusion::Transfusion(int i_myPlayerIndex) : Ability(i_myPlayerIndex, false, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {
    this->indicator = new PointAndClickIndicator(this->myPlayerIndex, this->range);

    lastRows = new int[positionsSavedCount];
    lastCols = new int[positionsSavedCount];
    for (int i = 0; i < positionsSavedCount; i++) {
        lastRows[i] = -1;
        lastCols[i] = -1;
    }
}
//constructor through networking
Transfusion::Transfusion(int i_myPlayerIndex, int i_targetPlayerIndex) : Ability(i_myPlayerIndex, true, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {
    this->targetPlayerIndex = i_targetPlayerIndex;//we dont know the indicator so its target has to be passed
    lastRows = new int[positionsSavedCount];
    lastCols = new int[positionsSavedCount];
    for (int i = 0; i < positionsSavedCount; i++) {
        lastRows[i] = -1;
        lastCols[i] = -1;
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
    float dist = Utils::calcDist2D(me->getCol() + halfW, target->getCol() + halfW,
        me->getRow() + halfH, target->getRow() + halfH);
    if (dist > range) {//if player is too far away
        tempGoalRow = target->getRow() + halfW;//find a path to his center because thats better than left top coords
        tempGoalCol = target->getCol() + halfH;
        GlobalRecources::pFinding->findPath(tempGoalCol, tempGoalRow, myPlayerIndex); //find a path to him
        abilityPathIndex = GlobalRecources::players[myPlayerIndex]->pathsFound;
    }
    else {
        nextPhase();//if already in range, just start casting without moving
    }
}

//find path to target player (damage part of succ)
void Transfusion::execute1() {
    int halfW = me->getWidth() / 2;
    if (tempGoalRow != (target->getRow() + halfW) || (tempGoalCol != target->getCol() + halfW)) {
        tempGoalRow = target->getRow() + halfW;
        tempGoalCol = target->getCol() + halfW;
        GlobalRecources::pFinding->findPath(tempGoalCol, tempGoalRow, myPlayerIndex);
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
        if (Utils::calcDist2D(me->getCol() + halfW, target->getCol() + halfW,
            me->getRow() + halfH, target->getRow() + halfH) < range) {
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
    tempGoalRow = target->getRow();
    tempGoalCol = target->getCol();
    int halfW = me->getWidth() / 2;
    bloodBall = new Projectile(me->getRow() + halfW, me->getCol() + halfW, velocity,
        tempGoalRow + halfW, tempGoalCol + halfW, false, radius, me);

    for (int i = 0; i < positionsSavedCount; i++) {
        lastRows[i] = -1;
        lastCols[i] = -1;
    }
}

void Transfusion::execute2() {
    checkBloodballCollision();
    findNewPathToPlayerTimer ++;
    if (findNewPathToPlayerTimer % 10 == true) {
        followPlayer();
    }

    bloodBall->move(GlobalRecources::worldRows, GlobalRecources::worldCols, nullptr, 0);//should go through walls so we just dont pass them
}

void Transfusion::draw2() {
    lastRows[cPositionSaveIndex] = bloodBall->getRow();
    lastCols[cPositionSaveIndex] = bloodBall->getCol();
    cPositionSaveIndex ++;
    if (cPositionSaveIndex >= positionsSavedCount) {
        cPositionSaveIndex = 0;
    }
    bloodBall->draw(sf::Color(200, 0, 0, 255));
    for (int i = 0; i < positionsSavedCount; i++) {
        if (lastRows[i] != -1) {
            Renderer::drawCircle(lastRows[i], lastCols[i], bloodBall->getRadius(), sf::Color(200, 0, 0, 255), true, 0, false);
        }
    }
}

void Transfusion::checkBloodballCollision() {
    Player* c = getBloodballTarget();
    //blood ball got to enemy and should fly back
    if (Utils::collisionRectCircle(c->getRow(), c->getCol(), c->getWidth(), c->getHeight(),
        bloodBall->getRow(), bloodBall->getCol(), bloodBall->getRadius(), 10) == true) {
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
    Player* c = getBloodballTarget();

    if (tempGoalRow != c->getRow() || tempGoalCol != c->getCol()) {
        tempGoalRow = c->getRow();
        tempGoalCol = c->getCol();
        int tempBBrow = bloodBall->getRow();
        int tempBBcol = bloodBall->getCol();


        int halfW = me->getWidth() / 2;
        delete bloodBall;//definitly exists at this point so we can delete it
        bloodBall = new Projectile(tempBBrow + radius, tempBBcol + radius, velocity,
            tempGoalRow + halfW, tempGoalCol + halfW, false, radius, me);
    }
}

Player* Transfusion::getBloodballTarget() {
    Player* c;
    if (flyBack == false) {
        c = target;
    }
    else {
        c = me;
    }
    return c;
}
