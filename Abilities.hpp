#include "Player.hpp"
#include "Renderer.hpp"
#include "Projectile.hpp"
#include "Terrain.hpp"
#include "Utils.hpp"
#include "PathfindingHandler.hpp"

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
using namespace abilityRecources;

class Fireball {
public:
    Fireball(int i_myPlayerIndex) {
        this->myPlayerI = i_myPlayerIndex;

        //Turn player to mouse coords and set mouse coords as goal coords
        this->goalRow = 0;
        this->goalCol = 0;
        Renderer::getMousePos(&goalCol, &goalRow, true);
        Player* myPlayer = players [myPlayerI];
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

        this->helpProjectile = new Projectile(startRow, startCol, velocity, goalRow, goalCol, radius, myPlayer);
    }

    void update() {
        if (this->exploding == false) {
            auto collidables = terrain->getCollidables();
           this-> helpProjectile->move(worldRows, worldCols, collidables->data(), collidables->size());
            //if the projectile reaches its max range or collides with anything, it should explode
            if (abs(this->startRow - this->helpProjectile->getRow()) * abs(this->startRow - this->helpProjectile->getRow())
                + abs(this->startCol - this->helpProjectile->getCol()) * abs(this->startCol - this->helpProjectile->getCol())
                > this->range * this->range || this->helpProjectile->isDead() == true) {
                this->exploding = true;
                this->fireStartTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
                
                this->explosionRow = this->helpProjectile->getRow() 
                + this->helpProjectile->getRadius() - this->explosionRange;
                this->explosionCol = this->helpProjectile->getCol() 
                + this->helpProjectile->getRadius() - this->explosionRange;
            }
        } 
        else {
            long cTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            this->timeDiff = cTime - this->fireStartTime.count();
            if (this->timeDiff < 2000) {
                for (int i = 0; i < playerCount; i++) {
                        Player* c = players [i];
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
        if (this->exploding == false) {
            this->helpProjectile->draw();
        }
        else {
            Renderer::drawCircle(this->explosionRow, this->explosionCol, this->explosionRange,
                sf::Color(255, 150, 0, 150), true, 0, false);
        }
    }


    //create from network input(row is just current row so even with lag the start is always synced)
    Fireball(int i_startRow, int i_startCol, int i_goalRow, int i_goalCol, int i_myPlayerIndex) {
        this->startRow = i_startRow;
        this->startCol = i_startCol;
        this->goalRow = i_goalRow;
        this->goalCol = i_goalCol;
        this-> myPlayerI = i_myPlayerIndex;

        this->helpProjectile = new Projectile(startRow, startCol, velocity, goalRow, goalCol, radius, players [myPlayerI]);
    }

public:
    int startRow;
    int startCol;
    int goalRow;
    int goalCol;
    bool finished = false;
    int myPlayerI;
private:
    bool dealtDamage = false;
    bool exploding = false;
    int explosionRange = 100;

    milliseconds fireStartTime;
    long timeDiff;
    int explosionRow, explosionCol;

    int explosionDmg = 50;
    float burnDmg = 0.5f;

    int radius = 50;
    int range = 500;
    float velocity = 10.0f;

    Projectile* helpProjectile;
};



//Point and click indicator class-------------------------------------------------------------------

class PointAndClickIndicator {
public:
    PointAndClickIndicator(int i_playerIndex, int i_range){
        this->myPlayerIndex = i_playerIndex;
        this->range = i_range;
    }

    void update() {
        sf::Cursor cursor;
        if (cursor.loadFromSystem(sf::Cursor::Cross)){
            Renderer::currentWindow->setMouseCursor(cursor);
        }

        //if true, end next tick (if no player was selected just do nothing and end indicator)

        for(int i = 0; i < playerCount; i++){
            if(i != this->myPlayerIndex) {
                Player* c = players [i];
                int mouseRow = 0;
                int mouseCol = 0;
                Renderer::getMousePos(&mouseCol, &mouseRow, true);
                if(Utils::collisionCoordsRect(c->getCol(), c->getRow(), c->getWidth(),
                        c->getHeight(), mouseCol, mouseRow) == true) {
                    //IF LEFTCLICK HAS BEEN PRESSED (see above) select player
                    if(sf::Mouse::isButtonPressed(sf::Mouse::Left)){
                        this->targetIndex = i;
                    }
                    //draw an outline around the hovered over player
                    Renderer::drawRectOutline(c->getRow(), c->getCol(), c->getWidth(),c->getHeight(),
                        sf::Color(75, 165, 180, 150), 5, false);
                } 
                else {
                    if(sf::Mouse::isButtonPressed(sf::Mouse::Left)){
                        endWOaction = true;
                    }
                }
            }
        }
    }

    void draw() {
        //draw range indicator
        int indicatorRow = players[this->myPlayerIndex]->getRow() + 
            players[this->myPlayerIndex]->getHeight()/ 2 - this->range;//range = radius of circle
        int indicatorCol = players[this->myPlayerIndex]->getCol() + 
            players[this->myPlayerIndex]->getWidth() / 2 - this->range;
        Renderer::drawCircle(indicatorRow, indicatorCol, this->range, sf::Color(0, 100, 100, 200), false, 10, false);
    }

    int getTargetIndex(){
        return this->targetIndex;
    }

    bool endWithoutAction(){
        return endWOaction;
    }
private:
    int myPlayerIndex;
    int range;

    int targetIndex = -1;
    bool endWOaction = false;
};



class Transfusion {
public:
    Transfusion(int i_myPlayerIndex) {
        this->myPlayerIndex = i_myPlayerIndex;
        indicator = new PointAndClickIndicator(this->myPlayerIndex, this->range);
    }

    void update() {
        if(casting == false){
            if(indicator->getTargetIndex() == -1) {
                indicator->update();
                if(indicator->endWithoutAction() == true) {
                    finishedWithoutCasting = true;
                }
            } 
            else {
                if(initializedEvents == false){
                    initializedEvents = true;
                    initEvents();
                }
                if(casting == false) {//if you still have to walk
                    bool stop = false;
                    //multithreading problem: we want, if another path is found for the player (e.g. through clicking)
                    //to stop going on the path the ability wants to find. But because of multithreading we cant say
                    //when the pathfinding is finished with this particular path, so we just count the paths up in the
                    //player obj and if the index is equal to abiltyPathIndex its still finding the path (same index
                    //as when it was saved) and if its one higher the path was found. if its 2 higher a new path
                    //was found and we interrupt.
                    if(players[myPlayerIndex]->pathsFound > abilityPathIndex + 1) {
                        stop = true;
                    }
                    if(stop == false) {
                        int halfW = me->getWidth() / 2;
                        int halfH = me->getHeight() / 2;
                        if(Utils::calcDist2D(me->getCol() + halfW, target->getCol() + halfW, 
                                    me->getRow() + halfH, target->getRow() + halfH) < range) {
                            //got into range, stop going on path an cast ability
                            players[myPlayerIndex]->deletePath();
                            casting = true;
                        }
                    } 
                    else {
                        //clicked somewhere else while finding path to target player to get in range => abort cast
                        finishedWithoutCasting = true;
                    }
                }
            }
        }
        else {
            if(castingInitialized == false) {
                castingInitialized = true;
                castStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
                
                tempGoalRow = target->getRow();
                tempGoalCol = target->getCol();
                int halfW = me->getWidth() / 2;
                bloodBall = new Projectile(me->getRow() + halfW, me->getCol() + halfW, velocity, 
                        tempGoalRow + halfW, tempGoalCol + halfW, radius, me);
            }

            long cTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            timeDiff = cTime - castStart;
            
            if(flyBack == false){
                //blood ball got to enemy and should fly back
                if (Utils::collisionRectCircle(target->getRow(), target->getCol(), target->getWidth(), target->getHeight(),
                            bloodBall->getRow(), bloodBall->getCol(), bloodBall->getRadius(), 10) == true) {
                    flyBack = true;
                    target->setHp(target->getHp() - dmg);
                }

                if(tempGoalRow != target->getRow() || tempGoalCol != target->getCol()){
                    tempGoalRow = target->getRow();
                    tempGoalCol = target->getCol();
                    int tempBBrow = bloodBall->getRow();
                    int tempBBcol = bloodBall->getCol();
                    delete bloodBall;//definitly exists at this point so we can delete it

                    int halfW = me->getWidth() / 2;
                    bloodBall = new Projectile(tempBBrow + radius, tempBBcol + radius, velocity,
                            tempGoalRow + halfW, tempGoalCol + halfW, radius, me);
                }
            } 
            else {
                //blood ball got back to player with hp
                if (Utils::collisionRectCircle(me->getRow(), me->getCol(), me->getWidth(), me->getHeight(),
                            bloodBall->getRow(), bloodBall->getCol(), bloodBall->getRadius(), radius) == true) {
                    if(me->getHp() + heal <= me->getMaxHp()){
                        me->setHp(me->getHp() + heal);
                    }

                    finishedCompletely = true;
                }

                if(tempGoalRow != me->getRow() || tempGoalCol != me->getCol()){
                    tempGoalRow = me->getRow();
                    tempGoalCol = me->getCol();
                    int tempBBrow = bloodBall->getRow();
                    int tempBBcol = bloodBall->getCol();
                    delete bloodBall;//definitly exists at this point so we can delete it

                    int halfW = me->getWidth() / 2;
                    bloodBall = new Projectile(tempBBrow + radius, tempBBcol + radius, velocity,
                            tempGoalRow + halfW, tempGoalCol + halfW, radius, me);
                }
            }
            bloodBall->move(worldRows, worldCols, nullptr, 0);//should go through walls so we just dont pass them
        }
    }

    void initEvents(){
        targetPlayer = indicator->getTargetIndex();
        me = players[myPlayerIndex];
        target = players[targetPlayer];

        finishedSelectingTarget = true;

        //if player out of range, run into range
        int halfW = me->getWidth() / 2;
        int halfH = me->getHeight() / 2;

        float dist = Utils::calcDist2D(me->getCol() + halfW, target->getCol() + halfW, 
                me->getRow() + halfH, target->getRow() + halfH);
        if(dist > range) {
            pFinding->findPath(target->getCol(), target->getRow(), myPlayerIndex);
            abilityPathIndex = players[myPlayerIndex]->pathsFound;
        } 
        else {
            casting = true;
        }
    }

    void draw() {
        if(indicator->getTargetIndex() == -1){
            indicator->draw();
        } 
        else {
            if(bloodBall != nullptr){
                bloodBall->draw();
            }
        }
    }

    bool finishedWithoutCasting = false;
    bool finishedCompletely = false;
    bool finishedSelectingTarget = false;
private:
    bool castingInitialized = false;
    long castStart;
    long timeDiff;
    float pathPercent;
    Projectile* bloodBall;
    int tempGoalRow, tempGoalCol;
    bool flyBack = false;

    PointAndClickIndicator* indicator;
    int range = 300;
    int myPlayerIndex;
    int targetPlayer;
    bool casting = false;
    bool initializedEvents = false;

    int abilityPathIndex;

    Player* me;
    Player* target;

    int dmg = 30;
    int heal = 15;
    int radius = 10;
    int velocity = 10.0f;
};