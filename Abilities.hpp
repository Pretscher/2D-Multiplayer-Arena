#include "Player.hpp"
#include "Renderer.hpp"
#include "Projectile.hpp"
#include "Terrain.hpp"
#include "Utils.hpp"
#include "PathfindingHandler.hpp"
#include "Indicator.hpp"
#include <math.h>
using namespace std::chrono;


class Ability {
public:
    Ability(int i_myPlayerIndex) {
        myPlayerIndex = i_myPlayerIndex;

        phaseCount = 5;
        currentPhase = 0;

        phaseInitialized = new bool[phaseCount];
        phaseStart = new long[phaseCount];
        phaseDuration = new int[phaseCount];
        timeBoundPhase = new bool [phaseCount];
        for (int i = 0; i < phaseCount; i++) {
            phaseInitialized [i] = false;
            timeBoundPhase [i] = false;
        }
    }

    void update() {
        if (finished == false) {
            if (currentPhase == 0) {
                checkTime();
                if (phaseInitialized [currentPhase] == false) {
                    init0();
                    phaseInitialized [currentPhase] = true;
                }
                execute0();
                return;
            }
            if (currentPhase == 1) {
                checkTime();
                if (phaseInitialized [currentPhase] == false) {
                    init1();
                    phaseInitialized [currentPhase] = true;
                }
                execute1();
                return;
            }
            if (currentPhase == 2) {
                checkTime();
                if (phaseInitialized [currentPhase] == false) {
                    init2();
                    phaseInitialized [currentPhase] = true;
                }
                execute2();
                return;
            }
            if (currentPhase == 3) {
                
                if (phaseInitialized [currentPhase] == false) {
                    init3();
                    phaseInitialized [currentPhase] = true;
                }
                execute3();
                return;
            }
            if (currentPhase == 4) {
                checkTime();
                if (phaseInitialized [currentPhase] == false) {
                    init4();
                    phaseInitialized [currentPhase] = true;
                }
                execute4();
                return;
            }
            if (currentPhase == 5) {
                finished = true;
            }
        }
    }

    void checkTime() {
        if (timeBoundPhase [currentPhase] == true) {
            auto cTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            int diff = cTime - phaseStart [currentPhase];
            if (diff > phaseDuration [currentPhase]) {
                nextPhase();
            }
        }
    }

    void draw() {
        if (currentPhase == 0) {
            if (phaseInitialized [currentPhase] == true) {
                draw0();
            }
        }
        if (currentPhase == 1) {
            if (phaseInitialized [currentPhase] == true) {
                draw1();
            }
        }
        if (currentPhase == 2) {
            if (phaseInitialized [currentPhase] == true) {
                draw2();
            }
        }
        if (currentPhase == 3) {
            if (phaseInitialized [currentPhase] == true) {
                draw3();
            }
        }
        if (currentPhase == 4) {
            if (phaseInitialized [currentPhase] == true) {
                draw4();
            }
        }
    }

    inline int getPhase() {
        return currentPhase;
    }

    inline void nextPhase() {
        currentPhase ++;
    }

    inline bool finishedPhase(int index) {
        return index < currentPhase;//if current Phase is higher than index, phase with index was finished
    }
    
    inline bool finishedCompletely() {
        return finished;
    }

    inline bool wasAddedToNetwork() {
        return addedToNetwork;
    }

    inline void addToNetwork() {
        addedToNetwork = true;
    }

    inline long getStartTime(int index) {
        return phaseStart [index];
    }

    //call this from the init-function of the to-be-time-limited phase
    inline void endPhaseAfterMS(int ms) {
        timeBoundPhase [currentPhase] = true;
        phaseDuration [currentPhase] = ms;
        phaseStart [currentPhase] = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }

    inline int getCastingPlayer() {
        return myPlayerIndex;
    }
public:
    bool finished;
protected:
    int myPlayerIndex;

    bool* phaseInitialized;

    bool* timeBoundPhase;
    long* phaseStart;
    int* phaseDuration;


    virtual void init0() { finished = true; }//if any of this is reached, finish. 
    virtual void init1() { finished = true; }
    virtual void init2() { finished = true; }
    virtual void init3() { finished = true; }
    virtual void init4() { finished = true; }

    virtual void execute0() { finished = true; }
    virtual void execute1() { finished = true; }
    virtual void execute2() { finished = true; }
    virtual void execute3() { finished = true; }
    virtual void execute4() { finished = true; }

    virtual void draw0() { finished = true; }
    virtual void draw1() { finished = true; }
    virtual void draw2() { finished = true; }
    virtual void draw3() { finished = true; }
    virtual void draw4() { finished = true; }

private:
    int currentPhase;
    int phaseCount;
    bool addedToNetwork;
};

namespace abilityRecources {

    int worldRows, worldCols;
    Terrain* terrain;
    Player** players;
    int playerCount;
    Pathfinding* pFinding;
    void init(Player** i_players, int i_playerCount, Terrain* i_terrain, int i_worldRows, int i_worldCols, Pathfinding* i_pathfinding) {
        players = i_players;
        playerCount = i_playerCount;
        terrain = i_terrain;
        worldRows = i_worldRows;
        worldCols = i_worldCols;
        pFinding = i_pathfinding;
    }
}



class Fireball : public Ability {
public:
    Fireball(int i_myPlayerIndex) : Ability(i_myPlayerIndex) {//both constructors are used
        indicator = new ProjectileIndicator(i_myPlayerIndex, this->range, this->radius, abilityRecources::playerCount, abilityRecources::players);
    }

    void init0() {

    }

    void execute0() {
        if (finishedNoCast == false) {
            if (indicator->endWithoutAction() == true) {
                delete indicator;
                finishedNoCast = true;
            }
            else if (indicator->destinationSelected() == true) {
                goalRow = indicator->getDestinationRow();
                goalCol = indicator->getDestinationCol();
                delete indicator;
                this->nextPhase();//init casting
            }
            else {
                indicator->update();
            }
        }
    }

    void init1() {
        Player* myPlayer = abilityRecources::players [myPlayerIndex];
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

    void execute1() {
        auto collidables = abilityRecources::terrain->getCollidables();
        this->helpProjectile->move(abilityRecources::worldRows, abilityRecources::worldCols, collidables->data(), collidables->size());
        //if the projectile reaches its max range or collides with anything, it should explode
        if ((abs(this->startRow - this->helpProjectile->getRow()) * abs(this->startRow - this->helpProjectile->getRow())
            + abs(this->startCol - this->helpProjectile->getCol()) * abs(this->startCol - this->helpProjectile->getCol())
                > this->range * this->range) || this->helpProjectile->isDead() == true) {

            this->nextPhase(); //init explosion
        }
    }

    void init2() {
        explosionDmg = 30;
        burnDmg = 0.25f;
        explosionDuration = 2000;
        dealtDamage = false;
        explosionRange = 80;

        if (connectedFireball == false) {//else time has to be synced with already ran out time
            endPhaseAfterMS(explosionDuration);
        }
        else {
            auto cTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            endPhaseAfterMS(explosionDuration - (cTime - tempTimeSinceExplosionStart));
        }

        this->explosionRow = this->helpProjectile->getRow() + this->helpProjectile->getRadius() - this->explosionRange;
        this->explosionCol = this->helpProjectile->getCol() + this->helpProjectile->getRadius() - this->explosionRange;
    }

    void execute2() {
        for (int i = 0; i < abilityRecources::playerCount; i++) {
            Player* c = abilityRecources::players [i];
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
        this->dealtDamage = true;
    }

    void draw0() {
        indicator->draw();
    }

    void draw1() {
        this->helpProjectile->draw(sf::Color(255, 120, 0, 255));
    }

    void draw2() {
        Renderer::drawCircle(this->explosionRow, this->explosionCol, this->explosionRange, sf::Color(255, 120, 0, 255), true, 0, false);
    }

    //create from network input(row is just current row so even with lag the start is always synced)
    Fireball(int i_currentRow, int i_currentCol, int i_goalRow, int i_goalCol, int i_myPlayerIndex, 
                           int i_phase, int i_timeSinceExplosionStart) : Ability(i_myPlayerIndex) {

        this->startRow = i_currentRow;
        this->startCol = i_currentCol;
        this->goalRow = i_goalRow;
        this->goalCol = i_goalCol;
        this->tempTimeSinceExplosionStart = i_timeSinceExplosionStart;
        //start explosion, only useful if you have a big lag and the fireball gets transmitted only 
        //after exploding or you connect after explosion
        
        this->helpProjectile = new Projectile(startRow, startCol, velocity, goalRow, goalCol, false, radius,
                                                                      abilityRecources::players[myPlayerIndex]);
        for (int i = 0; i < i_phase; i++) {
            nextPhase();
        }
        connectedFireball = true;
    }

    void limitGoalPosToRange() {
        float* vecToGoal = new float [2];
        vecToGoal [0] = goalCol - startCol;
        vecToGoal [1] = goalRow - startRow;
        //calculate vector lenght
        float lenght = sqrt((vecToGoal [0] * vecToGoal [0]) + (vecToGoal [1] * vecToGoal [1]));
        if (lenght > range) {
            //normalize vector lenght
            vecToGoal [0] /= lenght;
            vecToGoal [1] /= lenght;
            //stretch vector to range
            vecToGoal [0] *= range;
            vecToGoal [1] *= range;
            //place at starting point
            goalCol = startCol + vecToGoal [0];
            goalRow = startRow + vecToGoal [1];
        }
    }

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

    bool connectedFireball = false;
    bool finishedNoCast = false;

    int tempTimeSinceExplosionStart;
};




class Transfusion {
public:
    Transfusion(int i_myPlayerIndex) {
        this->myPlayerIndex = i_myPlayerIndex;
        this->indicator = new PointAndClickIndicator(this->myPlayerIndex, this->range, 
                abilityRecources::playerCount, abilityRecources::players);

        lastRows = new int [positionsSavedCount];
        lastCols = new int [positionsSavedCount];
        for (int i = 0; i < positionsSavedCount; i++) {
            lastRows [i] = -1;
            lastCols [i] = -1;
        }
    }
    //constructor through networking
    Transfusion(int i_myPlayerIndex, int i_targetPlayerIndex) {
        this->myPlayerIndex = i_myPlayerIndex;
        this->targetPlayerIndex = i_targetPlayerIndex;

        me = abilityRecources::players[myPlayerIndex];
        target = abilityRecources::players[targetPlayerIndex];
        casting = true;

        lastRows = new int [positionsSavedCount];
        lastCols = new int [positionsSavedCount];
        for (int i = 0; i < positionsSavedCount; i++) {
            lastRows [i] = -1;
            lastCols [i] = -1;
        }
    }

    void update() {
        if(casting == false) {
            if(indicator != nullptr && indicator->getTargetIndex() == -1) {
                indicator->update();
                if(indicator->endWithoutAction() == true) {
                    finishedWithoutCasting = true;
                    delete indicator;// we dont need this anymore
                }
            } 
            else {
                if(initializedEvents == false){
                    initializedEvents = true;
                    initCastAndRunInRange();
                }
                if(casting == false) {//if you still have to walk
                    int halfW = me->getWidth() / 2;
                    if(tempGoalRow != (target->getRow() + halfW) || (tempGoalCol != target->getCol() + halfW)) {
                        tempGoalRow = target->getRow() + halfW;
                        tempGoalCol = target->getCol() + halfW;
                        abilityRecources::pFinding->findPath(tempGoalCol, tempGoalRow, myPlayerIndex);
                        abilityPathIndex = abilityRecources::players[myPlayerIndex]->pathsFound;
                    }
                    bool stop = false;
                    //multithreading problem: we want, if another path is found for the player (e.g. through clicking)
                    //to stop going on the path the ability wants to find. But because of multithreading we cant say
                    //when the pathfinding is finished with this particular path, so we just count the paths up in the
                    //player obj and if the index is equal to abiltyPathIndex its still finding the path (same index
                    //as when it was saved) and if its one higher the path was found. if its 2 higher a new path
                    //was found and we interrupt.
                    if(abilityRecources::players[myPlayerIndex]->pathsFound > abilityPathIndex + 1) {
                        stop = true;
                    }
                    if(stop == false) {
                        int halfW = me->getWidth() / 2;
                        int halfH = me->getHeight() / 2;
                        if(Utils::calcDist2D(me->getCol() + halfW, target->getCol() + halfW, 
                                    me->getRow() + halfH, target->getRow() + halfH) < range) {
                            //got into range, stop going on path an cast ability
                            abilityRecources::players[myPlayerIndex]->deletePath();
                            casting = true;
                        }
                    } 
                    else {
                        //clicked somewhere else while finding path to target player to get in range => abort cast
                        finishedWithoutCasting = true;
                        delete indicator;// we dont need this anymore
                    }
                }
            }
        }
        else {
            if(castingInitialized == false) {
                castingInitialized = true;
                initCasting();
            }

            long cTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            timeDiff = cTime - castStart;


            checkBloodballCollision();
            findNewPathToPlayerTimer ++;
            if (findNewPathToPlayerTimer % 10 == true) {
                followPlayer();
            }

            bloodBall->move(abilityRecources::worldRows, abilityRecources::worldCols, nullptr, 0);//should go through walls so we just dont pass them
        }
    }

    void initCasting() {
        castStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

        tempGoalRow = target->getRow();
        tempGoalCol = target->getCol();
        int halfW = me->getWidth() / 2;
        bloodBall = new Projectile(me->getRow() + halfW, me->getCol() + halfW, velocity,
            tempGoalRow + halfW, tempGoalCol + halfW, false, radius, me);

        for (int i = 0; i < positionsSavedCount; i++) {
            lastRows [i] = -1;
            lastCols [i] = -1;
        }
    }

    void checkBloodballCollision() {
        Player* c = getBloodballTarget();
        //blood ball got to enemy and should fly back
        if (Utils::collisionRectCircle(c->getRow(), c->getCol(), c->getWidth(), c->getHeight(),
            bloodBall->getRow(), bloodBall->getCol(), bloodBall->getRadius(), 10) == true) {
            if (flyBack == false) {
                flyBack = true;
                target->setHp(target->getHp() - dmg);
            }
            else {
                if (me->getHp() + heal <= me->getMaxHp()) {
                    me->setHp(me->getHp() + heal);
                }
                else {
                    me->setHp(me->getMaxHp());
                }
                finishedCompletely = true;
            }
        }
    }

    void followPlayer() {
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

    Player* getBloodballTarget() {
        Player* c;
        if (flyBack == false) {
            c = target;
        }
        else {
            c = me;
        }
        return c;
    }

    void initCastAndRunInRange() {
        //save target player and unbind indicator from ability entirely
        targetPlayerIndex = indicator->getTargetIndex();
        me = abilityRecources::players[myPlayerIndex];
        target = abilityRecources::players[targetPlayerIndex];
        delete indicator;// we dont need this anymore
        indicator = nullptr;//we check this in drawing caus bools are for noobs
        finishedSelectingTarget = true;

        //if player out of range, run into range
        int halfW = me->getWidth() / 2;//we need this to calc the range between the player's coord centers
        int halfH = me->getHeight() / 2;
        float dist = Utils::calcDist2D(me->getCol() + halfW, target->getCol() + halfW, 
                me->getRow() + halfH, target->getRow() + halfH);
        if(dist > range) {//if player is too far away
            tempGoalRow = target->getRow() + halfW;//find a path to his center because thats better than left top coords
            tempGoalCol = target->getCol() + halfH;
            abilityRecources::pFinding->findPath(tempGoalCol, tempGoalRow, myPlayerIndex); //find a path to him
            abilityPathIndex = abilityRecources::players[myPlayerIndex]->pathsFound;
        } 
        else {
            casting = true;//if already in range, just start casting without moving
        }
    }

    void draw() {
        if(indicator != nullptr) {//set to nullptr if no longer needed
            indicator->draw();
        } 
        if(bloodBall != nullptr) {
            lastRows [cPositionSaveIndex] = bloodBall->getRow();
            lastCols [cPositionSaveIndex] = bloodBall->getCol();
            cPositionSaveIndex ++;
            if (cPositionSaveIndex >= positionsSavedCount) {
                cPositionSaveIndex = 0;
            }
            bloodBall->draw(sf::Color(255, 0, 0, 255));
            for (int i = 0; i < positionsSavedCount; i++) {
                if (lastRows [i] != -1) {
                    Renderer::drawCircle(lastRows [i], lastCols [i], bloodBall->getRadius(), sf::Color(255, 0, 0, 255), true, 0, false);
                }
            }
        }
    }

    //getters
    inline bool hasEndedNoCast() {
        return finishedWithoutCasting;
    }
    inline bool hasFinishedCast() {
        return finishedCompletely;
    }
    inline bool hasSelectedTarget() {
        return finishedSelectingTarget;
    }
    inline int getCastingPlayer() {
        return myPlayerIndex;
    }
    inline int getTargetPlayer() {
        return targetPlayerIndex;
    }
    inline bool isCasting() {
        return casting;
    }
    inline bool wasAddedToNetwork() {
        return addedToNetwork;
    }
    /* only want to set this to true so not param */
    void setAddedToNetwork() {
        addedToNetwork = true;
    }

private:
    bool castingInitialized = false;
    long castStart;
    long timeDiff;
    float pathPercent;
    Projectile* bloodBall = nullptr;//we check if it is a nullpr later
    int tempGoalRow, tempGoalCol;
    bool flyBack = false;//blood goes to target player and then flies back, we need a bool to determine this break point

    PointAndClickIndicator* indicator = nullptr;//we check if it is a nullpr later
    
    bool initializedEvents = false;

    int abilityPathIndex;

    Player* me;
    Player* target;
    //hardcoded stuff
    int dmg = 25;
    int heal = 25;
    int radius = 10;
    int velocity = 5.0f;
    int range = 300;

    int positionsSavedCount = 10;//amount of blood balls flowing around
    int* lastRows;
    int* lastCols;
    int cPositionSaveIndex = 0;

    //those have getters and setters and are important for outside management
    bool finishedWithoutCasting = false;
    bool finishedCompletely = false;
    bool finishedSelectingTarget = false;
    int myPlayerIndex;
    int targetPlayerIndex;
    bool casting = false;
    bool addedToNetwork = false;

    int findNewPathToPlayerTimer = 0;
};