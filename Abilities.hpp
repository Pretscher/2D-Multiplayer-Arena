#include "Player.hpp"
#include "Renderer.hpp"
#include "Projectile.hpp"
#include "Terrain.hpp"
#include "Utils.hpp"
#include "PathfindingHandler.hpp"
#include "Indicator.hpp"
#include <math.h>
using namespace std::chrono;

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


class Fireball {
public:
    Fireball(int i_myPlayerIndex) {
        this->myPlayerI = i_myPlayerIndex;
        indicator = new ProjectileIndicator(myPlayerI, this->range, this->radius, abilityRecources::playerCount, abilityRecources::players);
    }


    void initCast() {
        castingInitialized = true;

        Player* myPlayer = abilityRecources::players[myPlayerI];
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

    void update() {
        if (castingInitialized == false) {
            if (indicator->endWithoutAction() == true) {
                delete indicator;
                finishedWithoutCasting = true;
            }
            else if (indicator->destinationSelected() == true) {
                goalRow = indicator->getDestinationRow();
                goalCol = indicator->getDestinationCol();
                delete indicator;
                initCast();
            }
            else {
                indicator->update();
            }
        }
        else if (this->exploding == false) {
            auto collidables = abilityRecources::terrain->getCollidables();
            this->helpProjectile->move(abilityRecources::worldRows, abilityRecources::worldCols, collidables->data(), collidables->size());
            //if the projectile reaches its max range or collides with anything, it should explode
            if ((abs(this->startRow - this->helpProjectile->getRow()) * abs(this->startRow - this->helpProjectile->getRow())
                + abs(this->startCol - this->helpProjectile->getCol()) * abs(this->startCol - this->helpProjectile->getCol())
                > this->range * this->range) || this->helpProjectile->isDead() == true) {
                this->exploding = true;
                this->fireStartTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
                
                this->explosionRow = this->helpProjectile->getRow() 
                + this->helpProjectile->getRadius() - this->explosionRange;
                this->explosionCol = this->helpProjectile->getCol() 
                + this->helpProjectile->getRadius() - this->explosionRange;
            }
        } 
        else {
            long cTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            this->timeDiff = cTime - this->fireStartTime;
            if (this->timeDiff < explosionTime) {
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
            else {
                this->finished = true;
            }
        }
    }

    void draw() {
        if (finishedWithoutCasting == false) {//if the object is not waiting to be deleted
            if (castingInitialized == false) {
                indicator->draw();
            }
            else if (this->exploding == false) {
                this->helpProjectile->draw(sf::Color(255, 120, 0, 255));
            }
            else {
                Renderer::drawCircle(this->explosionRow, this->explosionCol, this->explosionRange,
                    sf::Color(255, 120, 0, 255), true, 0, false);
            }
        }
    }


    //create from network input(row is just current row so even with lag the start is always synced)
    Fireball(int i_currentRow, int i_currentCol, int i_goalRow, int i_goalCol, int i_myPlayerIndex, bool i_exploding, int i_timeSinceExplosionStart) {
        this->startRow = i_currentRow;
        this->startCol = i_currentCol;
        this->goalRow = i_goalRow;
        this->goalCol = i_goalCol;
        this->myPlayerI = i_myPlayerIndex;
        
        this->explosionRow = i_currentRow + this->radius - this->explosionRange;
        this->explosionCol = i_currentCol + this->radius - this->explosionRange;
        //start explosion, only useful if you have a big lag and the fireball gets transmitted only 
        //after exploding or you connect after explosion
        exploding = i_exploding;
        if (exploding == true) {
            long cTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            fireStartTime = cTime - i_timeSinceExplosionStart;
            castingInitialized = true;
        }
        //start cast
        else {

            limitGoalPosToRange();
            initCast();
        }
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
    inline bool finishedEverything() {
        return finished;
    }
    inline int getCastingPlayer() {
        return myPlayerI;
    }

    inline bool finishedNoCast() {
        return finishedWithoutCasting;
    }
    inline bool castingStarted() {
        return castingInitialized;
    }
    inline bool isExploding() {
        return exploding;
    }
    inline bool wasAddedToNetwork() {
        return addedToNetwork;
    }
    inline void addToNetwork() {
        addedToNetwork = true;
    }
    inline long getFireStartTime() {
        return fireStartTime;
    }
    inline int getProjectileRow() {
        return helpProjectile->getRow();
    }
    inline int getProjectileCol() {
        return helpProjectile->getCol();
    }

private:
    bool dealtDamage = false;
    int explosionRange = 80;


    long timeDiff;
    int explosionRow, explosionCol;
    int explosionTime = 2000;

    int explosionDmg = 30;
    float burnDmg = 0.25f;

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
    int myPlayerI;

    bool finished = false;
    bool finishedWithoutCasting = false;
    bool castingInitialized = false;
    bool addedToNetwork = false;
    bool exploding = false;
    long fireStartTime;

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