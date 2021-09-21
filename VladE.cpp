#include "VladE.hpp"
#include <chrono>
#include "Renderer.hpp"
#include "GlobalRecources.hpp"
#include "Utils.hpp"
#include "iostream" 
using namespace std;

static int i_onCDPhase = 0;
static int i_addToNetworkPhase = 0;
static int i_abilityIndex = 2;

VladE::VladE(int i_myPlayerIndex) : Ability(i_myPlayerIndex, false, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {
	endPhaseAfterMS(1500);
}
//constructor through networking
VladE::VladE() : Ability(NetworkCommunication::receiveNextToken(), true, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {
	int phase = NetworkCommunication::receiveNextToken();
	int timeInPhase = NetworkCommunication::receiveNextToken();
	if (phase == 0) {
		endPhaseAfterMS(1500 - timeInPhase);
	}
	if (phase == 1) {
		nextPhase();
		init1();//has to be called manually to use projectiles
		for (int i = 0; i < projectileCount; i++) {
			projectiles[i]->skipMovementTime(timeInPhase);
		}
	}
}

void VladE::execute0() {
	auto cTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
	float diff = cTime - phaseStart[0];
	percentFinishedCharge = diff / phaseDuration[0];
}

void VladE::draw0() {
	int innerCircleRadius = range * abs(1.0f - percentFinishedCharge);
	shared_ptr<Player> myPlayer = GlobalRecources::players->at(myPlayerIndex);

	int innerX = myPlayer->getX() + (myPlayer->getWidth() / 2) - innerCircleRadius;
	int innerY = myPlayer->getY() + (myPlayer->getHeight() / 2) - innerCircleRadius;
	Renderer::drawCircle(innerX, innerY, innerCircleRadius, sf::Color(150, 0, 0, 255), false, 5, false);

	int outerX = myPlayer->getX() + (myPlayer->getWidth() / 2) - range;
	int outerY = myPlayer->getY() + (myPlayer->getHeight() / 2) - range;
	Renderer::drawCircle(outerX, outerY, range, sf::Color(150, 0, 0, 255), false, 10, false);
}

void VladE::init1() {
	endPhaseAfterMS(1000);//we want to save the start time only till now and are too lazy to do it in a passed variable.
	projectileCount = 16;
	projectiles = new Projectile*[projectileCount];
	dealtDamageToPlayer = new bool[GlobalRecources::playerCount];
	
	for (int i = 0; i < GlobalRecources::playerCount; i++) {
		dealtDamageToPlayer[i] = false;
	}

	for (int i = 0; i < projectileCount; i++) {
		int startY;
		int startX;
		int goalY;
		int goalX;

		shared_ptr<Player> myPlayer = GlobalRecources::players->at(myPlayerIndex);
		int pX = myPlayer->getX() - 20;//spawn a bit away from player, with 20 distance around him
		int pY = myPlayer->getY() - 20;
		int pWidth = myPlayer->getWidth() + 40;
		int pHeight = myPlayer->getHeight() + 40;

		switch (i) {
		//diagonal paths
		case 0: //top left
			startY = pY;
			startX = pX;

			goalY = pY - 500;
			goalX = pX - 500;

			break;
		case 1://bottom left
			startY = pY + pHeight;
			startX = pX;

			goalY = pY + pHeight + 500;
			goalX = pX - 500;

			break;
		case 2://top right
			startY = pY;
			startX = pX + pWidth;

			goalY = pY - 500;
			goalX = pX + pWidth + 500;
			break;
		case 3://bottom right
			startY = pY + pHeight;
			startX = pX + pWidth;

			goalY = pY + pHeight + 500;
			goalX = pX + pWidth + 500;
			break;

		//horizontal paths
		case 4://left
			startY = pY + (pHeight / 2);
			startX = pX;

			goalY = pY + (pHeight / 2);
			goalX = pX - 500;
			break;
		case 5://right
			startY = pY;
			startX = pX + (pWidth / 2);

			goalY = pY - 500;
			goalX = pX + (pWidth / 2);
			break;
		case 6://top
			startY = pY + pHeight;
			startX = pX + (pWidth / 2);

			goalY = pY + pHeight + 500;
			goalX = pX + (pWidth / 2);
			break;
		case 7://bottom
			startY = pY + (pHeight / 2);
			startX = pX + pWidth;

			goalY = pY + (pHeight / 2);
			goalX = pX + pWidth + 500;
			break;


		//between diagonal and horizontal/vertical paths
		case 8://between right-bottom and bottom
			startY = pY + pHeight;
			startX = pX + ((3 * pHeight) / 4);

			goalY = pY + pHeight + 500;
			goalX = pX + ((3 * pHeight) / 4) + 250;
			break;
		case 9://between bottom and left-bottom
			startY = pY + pHeight;
			startX = pX + (pHeight / 4);

			goalY = pY + pHeight + 500;
			goalX = pX + (pHeight / 4) - 250;
			break;
		case 10://between left-bottom and left
			startY = pY + ((3 * pHeight) / 4);
			startX = pX;

			goalY = pY + ((3 * pHeight) / 4) + 250;
			goalX = pX - 500;
			break;
		case 11://between left and left-top
			startY = pY + (pHeight / 4);
			startX = pX;

			goalY = pY + (pHeight / 4) - 250;
			goalX = pX - 500;
			break;
		case 12://between left-top and top
			startY = pY;
			startX = pX + (pWidth / 4);

			goalY = pY - 500;
			goalX = pX + (pWidth / 4) - 250;
			break;
		case 13://between top and right-top
			startY = pY;
			startX = pX + ((3 * pWidth) / 4);

			goalY = pY - 500;
			goalX = pX + ((3 * pWidth) / 4) + 250;
			break;
		case 14://between right-top and right
			startY = pY + (pHeight / 4);
			startX = pX + pWidth;

			goalY = pY + (pHeight / 4) - 250;
			goalX = pX + pWidth + 500;
			break;
		case 15://between right and right-bottom
			startY = pY + ((3 * pHeight) / 4);
			startX = pX + pWidth;

			goalY = pY + ((3 * pHeight) / 4) + 250;
			goalX = pX + pWidth + 500;
			break;
		default:
			break;
		}


		limitPosToRange(&goalY, &goalX);

		if (goalX < 0) goalX = 0;
		if (goalY < 0) goalY = 0;
		projectiles[i] = new Projectile(startY, startX, velocity, goalY, goalX, false, radius, myPlayer);
	}
}

void VladE::execute1() {
	auto terrain = GlobalRecources::terrain->getCollidables();
	for (int i = 0; i < projectileCount; i++) {
		projectiles[i]->move(GlobalRecources::worldHeight, GlobalRecources::worldWidth, terrain->data(), terrain->size());
	}



	for (int j = 0; j < GlobalRecources::playerCount; j++) {
		for (int i = 0; i < projectileCount; i++) {
			if (projectiles[i]->getDead() == false) {
				if (j != myPlayerIndex) {
					shared_ptr<Player> cPlayer = GlobalRecources::players->at(j);
					if (cPlayer->targetAble == true) {
						if (Utils::colisionRectCircle(cPlayer->getY(), cPlayer->getX(), cPlayer->getWidth(), cPlayer->getHeight(),
							projectiles[i]->getY(), projectiles[i]->getX(), projectiles[i]->getRadius(), 10) == true) {
							projectiles[i]->setDead(true);
							if (dealtDamageToPlayer[j] == false) {
								cPlayer->setHp(cPlayer->getHp() - this->damage);
								dealtDamageToPlayer[j] = true;
							}
						}
					}
				}
			}
		}
	}

	//end ability after all projectiles reached their destination
	bool someoneAlive = false;
	for (int i = 0; i < projectileCount; i++) {
		if (projectiles[i]->getDead() == false) {
			someoneAlive = true;
			break;
		}
	}
	if (someoneAlive == false) {
		finished = true;
	}
}

void VladE::draw1() {
	for (int i = 0; i < projectileCount; i++) {
		if (projectiles[i]->getDead() == false) {
			projectiles[i]->draw(sf::Color(150, 0, 0, 255));
		}
	}
	shared_ptr<Player> myPlayer = GlobalRecources::players->at(myPlayerIndex);
	int outerY = myPlayer->getY() + (myPlayer->getHeight() / 2) - range;
	int outerX = myPlayer->getX() + (myPlayer->getWidth() / 2) - range;
	Renderer::drawCircle(outerX, outerY, range, sf::Color(150, 0, 0, 255), false, 10, false);
}

void VladE::init2() {
	finished = true;
}

void VladE::execute2() {

}

void VladE::draw2() {

}

void VladE::limitPosToRange(int* io_goalY, int* io_goalX) {
	shared_ptr<Player> myPlayer = GlobalRecources::players->at(myPlayerIndex);

	float* vecToGoal = new float[2];
	vecToGoal[0] = *io_goalX + radius - (myPlayer->getX() + (myPlayer->getWidth() / 2));
	vecToGoal[1] = *io_goalY + radius - (myPlayer->getY() + (myPlayer->getHeight() / 2));
	//calculate vector lenght
	float lenght = sqrt((vecToGoal[0] * vecToGoal[0]) + (vecToGoal[1] * vecToGoal[1]));
	if (lenght > range + radius) {
		//normalize vector lenght
		vecToGoal[0] /= lenght;
		vecToGoal[1] /= lenght;
		//stretch vector to range
		vecToGoal[0] *= range + radius;
		vecToGoal[1] *= range + radius;
		//place at starting point
		*io_goalX = myPlayer->getX() + vecToGoal[0] + radius;
		*io_goalY = myPlayer->getY() + vecToGoal[1] + radius;
	}
}

void VladE::send() {
	NetworkCommunication::addToken(this->myPlayerIndex);
	NetworkCommunication::addToken(this->getPhase());

	auto cTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
	int timeSinceStart = cTime - this->getStartTime(this->getPhase());
	NetworkCommunication::addToken(timeSinceStart);
}