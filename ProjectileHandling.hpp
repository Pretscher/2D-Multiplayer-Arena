#pragma once
#include <iostream>
#include "Renderer.hpp"
#include "Player.hpp"
#include "Projectile.hpp"
#include "Rect.hpp"
#include "Utils.hpp"
#include "NetworkCommunication.hpp"

class ProjectileHandling {
public:
	/**		int worldRows, int worldCols, Player** players, int myPlayerI, int playerCount
	**/
	ProjectileHandling(int worldRows, int worldCols, Player** players, int myPlayerI, int playerCount) {
		projectileVel = 10.0f;
		projectileRadius = 20;

		this->players = players;
		this->myPlayerI = myPlayerI;
		this->worldRows = worldRows;
		this->worldCols = worldCols;
		this->playerCount = playerCount;
		newProjectiles = new std::vector<Projectile*>();
		projectiles = new std::vector<Projectile*>();;//stores all projectiles for creation, drawing, moving and damage calculation. 
	}

	void update(Rect** collidables, int collidableSize) {
		//dont shoot a projectile for the same space-press
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) == false) {
			samePress = false;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) == true && samePress == false) {
			samePress = true;


			int mouseX = -1, mouseY = -1;
			Renderer::getMousePos(&mouseX, &mouseY, true);//writes mouse coords into mouseX, mouseY
			//calculates a function between these points and moves on it

			Player* myPlayer = players[myPlayerI];

			int row = 0, col = 0;
			int halfW = myPlayer->getWidth() / 2;
			int halfH = myPlayer->getHeight() / 2;


			//if projectile distination is above player
			if (mouseY < myPlayer->getRow()) {
				col = myPlayer->getCol() + halfW;
				row = myPlayer->getRow();
				myPlayer->setTexture(2);
			}
			//below
			if (mouseY > myPlayer->getRow()) {
				col = myPlayer->getCol() + halfW;
				row = myPlayer->getRow() + myPlayer->getHeight();
				myPlayer->setTexture(3);
			}



			Projectile* p = new Projectile(row, col, projectileVel, mouseY, mouseX, projectileRadius, myPlayer);
			projectiles->push_back(p);
			newProjectiles->push_back(p);
		}

		//move projectiles (we loop through em in drawingLoop too but later it will be in a different thread so we cant use the same loop)
		for (int i = 0; i < projectiles->size(); i++) {
			Projectile* p = projectiles->at(i);
			p->move(worldRows, worldCols, collidables, collidableSize);//give it the maximum rows so it know when it can stop moving

			for (int j = 0; j < playerCount; j++) {
				Player* cPlayer = players[j];
				if (cPlayer != p->getPlayer()) {
					if (players[j]->getHp() > 0) {

						if (Utils::collisionRectCircle(cPlayer->getRow(), cPlayer->getCol(), cPlayer->getWidth(), cPlayer->getHeight(),
							p->getRow(), p->getCol(), p->getRadius(), 10) == true) {
							p->setDead(true);
							cPlayer->setHp(cPlayer->getHp() - p->getPlayer()->getDmg());
						}
					}
				}
			}

			if (p->isDead() == true) {
				projectiles->erase(projectiles->begin() + i);//delete projecile if dead
			}
		}
	}

	void draw() {
		for (int i = 0; i < projectiles->size(); i++) {
			projectiles->at(i)->draw();
		}
	}

	
	void sendProjectiles() {
		int networkingStart = NetworkCommunication::getTokenCount();
		NetworkCommunication::addToken(networkingStart);
		int networkingEnd = networkingStart + (4 * newProjectiles->size());
		if (newProjectiles->size() > 0) networkingEnd--;
		NetworkCommunication::addToken(networkingEnd);
		
		for (int i = 0; i < newProjectiles->size(); i++) {
			Projectile* current = newProjectiles->at(i);

			NetworkCommunication::addToken((int)current->getRow());//natively floats for half movements smaller than a row/col
			NetworkCommunication::addToken((int)current->getCol());
			NetworkCommunication::addToken(current->getGoalRow());
			NetworkCommunication::addToken(current->getGoalCol());
		}
		if (newProjectiles->size() > 0) {
			int networkingEnd = NetworkCommunication::getTokenCount() - 1;
			NetworkCommunication::addToken(networkingEnd);
		}
		newProjectiles->clear();
	}

	void receiveProjectiles() {
		int otherPlayerI = 0;
		if(myPlayerI == 0) {
			otherPlayerI = 1;
		}
		int counter = 0;
		int row = 0;
		int col = 0;
		int goalRow = 0;
		int goalCol = 0;

		int networkingStart = NetworkCommunication::receiveNextToken();
		int networkingEnd = NetworkCommunication::receiveNextToken();
		for (int i = networkingStart; i < networkingEnd; i++) {
			switch (counter) {
			case 0:
				row = NetworkCommunication::receiveNextToken();
			case 1:
				col = NetworkCommunication::receiveNextToken();
			case 2:
				goalRow = NetworkCommunication::receiveNextToken();//keep important decimal places through */ 10000
			case 3:
				goalCol = NetworkCommunication::receiveNextToken();
			default:;//do nothing on default, either all 4 cases are given or none, nothing else can happen
			}
			counter++;
			if (counter > 3) {
				projectiles->push_back(new Projectile(row, col, projectileVel, goalRow, goalCol, 
					projectileRadius,  players[otherPlayerI]));
				counter = 0;
			}
		}
	}

private:
	float projectileVel;
	int projectileRadius;
	bool samePress = false;
	std::vector<Projectile*>* newProjectiles;
	std::vector<Projectile*>* projectiles;//stores all projectiles for creation, drawing, moving and damage calculation. 

	int worldRows, worldCols;
	Player** players;
	int myPlayerI;
	int playerCount;
};