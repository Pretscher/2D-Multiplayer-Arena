#include "Player.hpp"
#include "Renderer.hpp"
#include "Projectile.hpp"
#include "Terrain.hpp"
#include "Utils.hpp"
#include <bits/stdc++.h> 

using namespace std::chrono;

namespace abilityRecources {

    int worldRows, worldCols;
    Terrain* terrain;
    Player** players;
    int playerCount;
    void init(Player** i_players, int i_playerCount, Terrain* i_terrain, int i_worldRows, int i_worldCols) {
        players = i_players;
        playerCount = i_playerCount;
        terrain = i_terrain;
        worldRows = i_worldRows;
        worldCols = i_worldCols;
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
                this->dead = true;
            }
        }
    }

    void draw() {
        if (this->exploding == false) {
            this->helpProjectile->draw();
        }
        else {
            Renderer::drawCircle(this->explosionRow, this->explosionCol, this->explosionRange,
                sf::Color(255, 150, 0, 150), true, false);
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
    bool dead = false;
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
        if(sf::Mouse::isButtonPressed(sf::Mouse::Left)){
            for(int i = 0; i < playerCount; i++){
                if(i != this->myPlayerIndex) {
                    Player* c = players [i];
                    int mouseRow = 0;
                    int mouseCol = 0;
                    Renderer::getMousePos(&mouseCol, &mouseRow, true);
                    if(Utils::collisionCoordsRect(c->getCol(), c->getRow(), c->getWidth(),
                        c->getHeight(), mouseCol, mouseRow) == true){
                        this->targetIndex = i;
                    }
                }
            }
        }
    }

    void draw() {
        //draw range indicator
        int indicatorRow = players[this->myPlayerIndex]->getRow() - this->range;//range = radius of circle
        int indicatorCol = players[this->myPlayerIndex]->getCol() - this->range;
        Renderer::drawCircle(indicatorRow, indicatorCol, this->range, sf::Color(0, 50, 100, 255), false, false);
    }

    int getTargetIndex(){
        return this->targetIndex;
    }
private:
    int myPlayerIndex;
    int range;

    int targetIndex = -1;

};



class Transfusion {
public:
    Transfusion(int i_myPlayerIndex) {
        this->myPlayerIndex = i_myPlayerIndex;
        indicator = new PointAndClickIndicator(this->myPlayerIndex, this->range);
    }

    void update() {
        if(indicator->getTargetIndex() == -1){
            indicator->update();
        } 
        else {

        }
    }

    void draw() {
        if(indicator->getTargetIndex() == -1){
            indicator->draw();
        } 
        else {

        }
    }

private:

    PointAndClickIndicator* indicator;
    int range = 300;
    int myPlayerIndex;
};