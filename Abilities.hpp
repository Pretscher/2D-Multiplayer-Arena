#include "Player.hpp"
#include "Renderer.hpp"
#include "Projectile.hpp"
#include "Terrain.hpp"
#include "Utils.hpp"

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
    bool dead = false;
    Fireball(int i_myPlayerIndex) {
        this->myPlayerI = i_myPlayerIndex;

        //hardcoded stuff 
        float velocity = 10.0f;
        radius = 50;
        range = 500;

        explosionRange = 100;
        explosionDmg = 50;
        burnDmg = 0.5f;
        //Turn player to mouse coords and set mouse coords as goal coords
        goalRow = 0;
        goalCol = 0;
        Renderer::getMousePos(&goalCol, &goalRow, true);
        Player* myPlayer = players [myPlayerI];
        //if projectile destination is above player
        if (goalRow < myPlayer->getRow()) {
            startCol = myPlayer->getCol() + (myPlayer->getWidth() / 2);
            startRow = myPlayer->getRow();
            myPlayer->setTexture(2);
        }
        //below
        if (goalRow > myPlayer->getRow()) {
            startCol = myPlayer->getCol() + (myPlayer->getWidth() / 2);
            startRow = myPlayer->getRow() + myPlayer->getHeight();
            myPlayer->setTexture(3);
        }

        helpProjectile = new Projectile(startRow, startCol, velocity, goalRow, goalCol, radius, myPlayer);
    }

    void update() {
        if (exploding == false) {
            auto collidables = terrain->getCollidables();
            helpProjectile->move(worldRows, worldCols, collidables->data(), collidables->size());
            //if the projectile reaches its max range or collides with anything, it should explode
            if (abs(startRow - helpProjectile->getRow()) * abs(startRow - helpProjectile->getRow())
                + abs(startCol - helpProjectile->getCol()) * abs(startCol - helpProjectile->getCol())
                > range * range || helpProjectile->isDead() == true) {
                exploding = true;
                fadingTimer = 255;
                explosionRow = helpProjectile->getRow() + helpProjectile->getRadius() - explosionRange;
                explosionCol = helpProjectile->getCol() + helpProjectile->getRadius() - explosionRange;
            }
        } 
        else {
            fadingTimer--;
            if (fadingTimer > 0) {
                for (int i = 0; i < playerCount; i++) {
                    if (i != myPlayerI) {
                        Player* c = players [i];
                        if (Utils::collisionRectCircle(c->getCol(), c->getRow(), c->getWidth(), c->getHeight(),
                            explosionRow, explosionCol, explosionRange, 5) == true) {
                            if (dealtDamage == false) {
                                c->setHp(c->getHp() - explosionDmg);
                            }
                            else {
                                c->setHp(c->getHp() - burnDmg);
                            }
                        }
                    }
                }
                dealtDamage = true;
            }
            else {
                dead = true;
            }
        }
    }

    void draw() {
        if (exploding == false) {
            helpProjectile->draw();
        }
        else {
            Renderer::drawCircle(helpProjectile->getRow() + helpProjectile->getRadius() - explosionRange, 
                helpProjectile->getCol() + helpProjectile->getRadius() - explosionRange, explosionRange,
                sf::Color(255, 150, 0, fadingTimer), true, false);
        }
    }

private:
    bool dealtDamage = false;
    bool exploding = false;
    int explosionRange;
    int fadingTimer;
    int explosionRow, explosionCol;

    int explosionDmg;
    float burnDmg;

    int radius;

    int range;
    int startRow;
    int startCol;
    int goalRow;
    int goalCol;
    Projectile* helpProjectile;
    int myPlayerI;
};