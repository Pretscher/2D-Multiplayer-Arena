#include "Player.hpp"
#include "Renderer.hpp"
#include "Projectile.hpp"

class Ability {
public: 
    Ability(Player** i_players, int i_playerCount, int i_myPlayerIndex){
        this->players = i_players;
        this->playerCount = i_playerCount;
        this->myPlayerI = i_myPlayerIndex;
    }
protected:
    Player** players;
    int playerCount;
    int myPlayerI;
    
    int dmg;
    int row, col;
};

class Fireball:public Ability {
public:
    void init() {
        //hardcoded stuff 
        float velocity = 0.1f;
        int radius = 50;
        dmg = 50;
        //Turn player to mouse coords and set mouse coords as goal coords
        goalRow = 0;
        goalCol = 0;
        startRow = row;
        startCol = col;
        Renderer::getMousePos(&goalCol, &goalRow, true);
        Player* myPlayer = players[myPlayerI];
        //if projectile destination is above player
		if (goalRow < myPlayer->getRow()) {
			col = myPlayer->getCol() + (myPlayer->getWidth() / 2);
			row = myPlayer->getRow();
			myPlayer->setTexture(2);
		}
		//below
		if (goalRow > myPlayer->getRow()) {
			col = myPlayer->getCol() + (myPlayer->getWidth() / 2);
			row = myPlayer->getRow() + myPlayer->getHeight();
			myPlayer->setTexture(3);
		}

        helpProjectile = new Projectile(row, col, velocity, goalRow, goalCol, radius, myPlayer);
    }

private:
    int startRow;
    int startCol;
    int goalRow;
    int goalCol;
    Projectile* helpProjectile;
};