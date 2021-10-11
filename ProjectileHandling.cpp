#include "ProjectileHandling.hpp"

#include "Renderer.hpp"
#include "Utils.hpp"//colision between projectiles and players/terrain calculated in utils
#include "NetworkCommunication.hpp"//send and receive stuff through networking
#include "GlobalRecources.hpp"

#include "iostream" 
using namespace std;
ProjectileHandling::ProjectileHandling() {
	projectileVel = 10.0f;
	projectileRadius = 20;

	this->players = GlobalRecources::players;

	this->worldHeight = GlobalRecources::worldHeight;
	this->worldWidth = GlobalRecources::worldWidth;
	this->playerCount = GlobalRecources::playerCount;
	newProjectiles = vector<shared_ptr<Projectile>>();
	projectiles = vector<shared_ptr<Projectile>>();//stores all projectiles for creation, drawing, moving and damage calculation. 
}

void ProjectileHandling::update(const shared_ptr<vector<Rect>> collidables) {
	//dont shoot a projectile for the same space-press
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) == false) {
		samePress = false;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) == true && samePress == false) {
		samePress = true;


		int mouseX, mouseY;
		Renderer:: getMousePos(mouseX, mouseY, true, true);//writes mouse coords into mouseX, mouseY
		//calculates a function between these points and moves on it

		shared_ptr<Player> myPlayer = players->at(myPlayerI);//we will modify this pointer here. 

		int y = 0, x = 0;
		int halfW = myPlayer->getWidth() / 2;
		int halfH = myPlayer->getHeight() / 2;


		//if projectile destination is above player
		if (mouseY < myPlayer->getY()) {
			x = myPlayer->getX() + halfW;
			y = myPlayer->getY();
			myPlayer->setTexture(2);
		}
		//below
		if (mouseY > myPlayer->getY()) {
			x = myPlayer->getX() + halfW;
			y = myPlayer->getY() + myPlayer->getHeight();
			myPlayer->setTexture(3);
		}



		shared_ptr<Projectile> p(new Projectile(y, x, projectileVel, mouseY, mouseX, true, projectileRadius, myPlayer));
		projectiles.push_back(p);
		newProjectiles.push_back(p);
	}

	//move projectiles (we loop through em in drawingLoop too but later it will be in a different thread so we cant use the same loop)
	for (int i = 0; i < projectiles.size(); i++) {
		const Projectile* p = projectiles.at(i).get();
		projectiles.at(i)->move(worldHeight, worldWidth, collidables);//give it the maximum ys so it know when it can stop moving

		for (int j = 0; j < playerCount; j++) {
			const Player* cPlayer = players->at(j).get();//read only to shorten names
			if (cPlayer->targetAble == true) {
				if (cPlayer != p->getPlayer().get()) {
					if (players->at(j)->getHp() > 0) {

						if (Utils::colisionRectCircle(cPlayer->getY(), cPlayer->getX(), cPlayer->getWidth(), cPlayer->getHeight(),
							p->getY(), p->getX(), p->getRadius(), 10) == true) {
							projectiles.at(i)->setDead(true);
							players->at(j)->setHp(cPlayer->getHp() - p->getPlayer()->getDmg());
						}
					}
				}
			}
		}

		if (p->isDead() == true) {
			for (int k = 0; k < newProjectiles.size(); k++) {
				if (newProjectiles.at(k) == projectiles.at(i)) {
					newProjectiles.erase(newProjectiles.begin() + k);//delete projecile if dead
				}
			}
			projectiles.erase(projectiles.begin() + i);//delete projecile if dead
		}
	}
}

void ProjectileHandling::draw() {
	for (int i = 0; i < projectiles.size(); i++) {
		projectiles.at(i)->draw(sf::Color(100, 100, 100, 255));
	}
}


void ProjectileHandling::sendProjectiles(int socketIndex) {
	int networkingStart = NetworkCommunication::getTokenCount(socketIndex);
	NetworkCommunication::addToken(networkingStart);
	int networkingEnd = networkingStart + (4 * newProjectiles.size());
	NetworkCommunication::addToken(networkingEnd);

	for (int i = 0; i < newProjectiles.size(); i++) {
		const Projectile* current = newProjectiles.at(i).get();

		NetworkCommunication::addToken((int)current->getY());//natively floats for half movements smaller than a y/x
		NetworkCommunication::addToken((int)current->getX());
		NetworkCommunication::addToken(current->getGoalY());
		NetworkCommunication::addToken(current->getGoalX());
	}
	if (newProjectiles.size() > 0) {
		int networkingEnd = NetworkCommunication::getTokenCount(socketIndex) - 1;
		NetworkCommunication::addToken(networkingEnd);
	}
	newProjectiles.clear();
}

void ProjectileHandling::receiveProjectiles(int index) {
	int otherPlayerI = 0;
	if (myPlayerI == 0) {
		otherPlayerI = 1;
	}
	int counter = 0;
	int y = 0;
	int x = 0;
	int goalY = 0;
	int goalX = 0;

	int networkingStart = NetworkCommunication::receiveNextToken(index);
	int networkingEnd = NetworkCommunication::receiveNextToken(index);
	for (int i = networkingStart; i < networkingEnd; i++) {
		switch (counter) {
		case 0:
			y = NetworkCommunication::receiveNextToken(index);
			break;
		case 1:
			x = NetworkCommunication::receiveNextToken(index);
			break;
		case 2:
			goalY = NetworkCommunication::receiveNextToken(index);//keep important decimal places through */ 10000
			break;
		case 3:
			goalX = NetworkCommunication::receiveNextToken(index);
			break;
		default:;//do nothing on default, either all 4 cases are given or none, nothing else can happen
		}
		counter++;
		if (counter > 3) {
			projectiles.push_back(shared_ptr<Projectile>(new Projectile(y, x, projectileVel, goalY, goalX, true,
				projectileRadius, players->at(otherPlayerI))));
			counter = 0;
		}
	}
}
