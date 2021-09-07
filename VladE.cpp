#include "VladE.hpp"
#include <chrono>
#include "Renderer.hpp"
#include "GlobalRecources.hpp"
#include "Utils.hpp"


static int i_onCDPhase = 1;
static int i_addToNetworkPhase = 0;
static int i_abilityIndex = 2;

VladE::VladE(int i_myPlayerIndex) : Ability(i_myPlayerIndex, false, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {
	endPhaseAfterMS(1500);
}
//constructor through networking
VladE::VladE(int i_myPlayerIndex, int i_phase, int i_timeInPhase) : Ability(i_myPlayerIndex, true, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {
	if (i_phase == 0) {
		endPhaseAfterMS(1500 - i_timeInPhase);
	}
	if (i_phase == 1) {
		nextPhase();
		init1();//has to be called manually to use projectiles
		for (int i = 0; i < projectileCount; i++) {
			projectiles [i]->skipMovementTime(i_timeInPhase);
		}
	}
}

void VladE::execute0() {
	auto cTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	float diff = cTime - phaseStart [0];
	percentFinishedCharge = diff / phaseDuration [0];
}

void VladE::draw0() {
	int innerCircleRadius = range * abs(1.0f - percentFinishedCharge);
	Player* myPlayer = GlobalRecources::players [myPlayerIndex];

	int innerRow = myPlayer->getRow() + (myPlayer->getHeight() / 2) - innerCircleRadius;
	int innerCol = myPlayer->getCol() + (myPlayer->getWidth() / 2) - innerCircleRadius;
	Renderer::drawCircle(innerRow, innerCol, innerCircleRadius, sf::Color(150, 0, 0, 255), false, 5, false);
	
	int outerRow = myPlayer->getRow() + (myPlayer->getHeight() / 2) - range;
	int outerCol = myPlayer->getCol() + (myPlayer->getWidth() / 2) - range;
	Renderer::drawCircle(outerRow, outerCol, range, sf::Color(150, 0, 0, 255), false, 10, false);
}

void VladE::init1() {
	endPhaseAfterMS(1000);//we want to save the start time only till now and are too lazy to do it in a passed variable.
	projectileCount = 16;
	projectiles = new Projectile* [projectileCount];
	dealtDamageToPlayer = new bool [GlobalRecources::playerCount];
	
	for (int i = 0; i < GlobalRecources::playerCount; i++) {
		dealtDamageToPlayer [i] = false;
	}

	for (int i = 0; i < projectileCount; i++) {
		int startRow;
		int startCol;
		int goalRow;
		int goalCol;

		Player* myPlayer = GlobalRecources::players [myPlayerIndex];
		int pCol = myPlayer->getCol() - 20;//spawn a bit away from player, with 20 distance around him
		int pRow = myPlayer->getRow() - 20;
		int pWidth = myPlayer->getWidth() + 40;
		int pHeight = myPlayer->getHeight() + 40;

		switch (i) {
		//diagonal paths
		case 0: //top left
			startRow = pRow;
			startCol = pCol;

			goalRow = pRow - 500;
			goalCol = pCol - 500;

			break;
		case 1://bottom left
			startRow = pRow + pHeight;
			startCol = pCol;

			goalRow = pRow + pHeight + 500;
			goalCol = pCol - 500;

			break;
		case 2://top right
			startRow = pRow;
			startCol = pCol + pWidth;

			goalRow = pRow - 500;
			goalCol = pCol + pWidth + 500;
			break;
		case 3://bottom right
			startRow = pRow + pHeight;
			startCol = pCol + pWidth;

			goalRow = pRow + pHeight + 500;
			goalCol = pCol + pWidth + 500;
			break;

		//horizontal paths
		case 4://left
			startRow = pRow + (pHeight / 2);
			startCol = pCol;

			goalRow = pRow + (pHeight / 2);
			goalCol = pCol - 500;
			break;
		case 5://right
			startRow = pRow;
			startCol = pCol + (pWidth / 2);

			goalRow = pRow - 500;
			goalCol = pCol + (pWidth / 2);
			break;
		case 6://top
			startRow = pRow + pHeight;
			startCol = pCol + (pWidth / 2);

			goalRow = pRow + pHeight + 500;
			goalCol = pCol + (pWidth / 2);
			break;
		case 7://bottom
			startRow = pRow + (pHeight / 2);
			startCol = pCol + pWidth;

			goalRow = pRow + (pHeight / 2);
			goalCol = pCol + pWidth + 500;
			break;


		//between diagonal and horizontal/vertical paths
		case 8://between right-bottom and bottom
			startRow = pRow + pHeight;
			startCol = pCol + ((3 * pHeight) / 4);

			goalRow = pRow + pHeight + 500;
			goalCol = pCol + ((3 * pHeight) / 4) + 250;
			break;
		case 9://between bottom and left-bottom
			startRow = pRow + pHeight;
			startCol = pCol + (pHeight / 4);

			goalRow = pRow + pHeight + 500;
			goalCol = pCol + (pHeight / 4) - 250;
			break;
		case 10://between left-bottom and left
			startRow = pRow + ((3 * pHeight) / 4);
			startCol = pCol;

			goalRow = pRow + ((3 * pHeight) / 4) + 250;
			goalCol = pCol - 500;
			break;
		case 11://between left and left-top
			startRow = pRow + (pHeight / 4);
			startCol = pCol;

			goalRow = pRow + (pHeight / 4) - 250;
			goalCol = pCol - 500;
			break;
		case 12://between left-top and top
			startRow = pRow;
			startCol = pCol + (pWidth / 4);

			goalRow = pRow - 500;
			goalCol = pCol + (pWidth / 4) - 250;
			break;
		case 13://between top and right-top
			startRow = pRow;
			startCol = pCol + ((3 * pWidth) / 4);

			goalRow = pRow - 500;
			goalCol = pCol + ((3 * pWidth) / 4) + 250;
			break;
		case 14://between right-top and right
			startRow = pRow + (pHeight / 4);
			startCol = pCol + pWidth;

			goalRow = pRow + (pHeight / 4) - 250;
			goalCol = pCol + pWidth + 500;
			break;
		case 15://between right and right-bottom
			startRow = pRow + ((3 * pHeight) / 4);
			startCol = pCol + pWidth;

			goalRow = pRow + ((3 * pHeight) / 4) + 250;
			goalCol = pCol + pWidth + 500;
			break;
		default:
			break;
		}


		limitPosToRange(&goalRow, &goalCol);

		if (goalCol < 0) goalCol = 0;
		if (goalRow < 0) goalRow = 0;
		projectiles [i] = new Projectile(startRow, startCol, velocity, goalRow, goalCol, false, radius, myPlayer);
	}
}

void VladE::execute1() {
	auto terrain = GlobalRecources::terrain->getCollidables();
	for (int i = 0; i < projectileCount; i++) {
		projectiles[i]->move(GlobalRecources::worldRows, GlobalRecources::worldCols, terrain->data(), terrain->size());
	}


	//end ability after all projectiles reached their destination
	bool someoneAlive = false;
	for (int j = 0; j < GlobalRecources::playerCount; j++) {
		for (int i = 0; i < projectileCount; i++) {
			if (projectiles [i]->getDead() == false) {
				if (j != myPlayerIndex) {
					Player* cPlayer = GlobalRecources::players [j];
					if (Utils::collisionRectCircle(cPlayer->getRow(), cPlayer->getCol(), cPlayer->getWidth(), cPlayer->getHeight(),
						projectiles [i]->getRow(), projectiles [i]->getCol(), projectiles [i]->getRadius(), 10) == true) {
						projectiles [i]->setDead(true);
						if (dealtDamageToPlayer[j] == false) {
							cPlayer->setHp(cPlayer->getHp() - this->damage);
							dealtDamageToPlayer [j] = true;
						}
					}
				}
			}
		}
	}

	for (int i = 0; i < projectileCount; i++) {
		if (projectiles [i]->getDead() == false) {
			someoneAlive = true;
			break;
		}
		if (someoneAlive == false) {
			finished = true;
		}
	}
}

void VladE::draw1() {
	for (int i = 0; i < projectileCount; i++) {
		if (projectiles [i]->getDead() == false) {
			projectiles [i]->draw(sf::Color(150, 0, 0, 255));
		}
	}
	Player* myPlayer = GlobalRecources::players [myPlayerIndex];
	int outerRow = myPlayer->getRow() + (myPlayer->getHeight() / 2) - range;
	int outerCol = myPlayer->getCol() + (myPlayer->getWidth() / 2) - range;
	Renderer::drawCircle(outerRow, outerCol, range, sf::Color(150, 0, 0, 255), false, 10, false);
}

void VladE::init2() {
	finished = true;
}

void VladE::execute2() {

}

void VladE::draw2() {

}

void VladE::limitPosToRange(int* io_goalRow, int* io_goalCol) {
	Player* myPlayer = GlobalRecources::players [myPlayerIndex];

	float* vecToGoal = new float [2];
	vecToGoal [0] = *io_goalCol + radius - (myPlayer->getCol() + (myPlayer->getWidth() / 2));
	vecToGoal [1] = *io_goalRow + radius - (myPlayer->getRow() + (myPlayer->getHeight() / 2));
	//calculate vector lenght
	float lenght = sqrt((vecToGoal [0] * vecToGoal [0]) + (vecToGoal [1] * vecToGoal [1]));
	if (lenght > range + radius) {
		//normalize vector lenght
		vecToGoal [0] /= lenght;
		vecToGoal [1] /= lenght;
		//stretch vector to range
		vecToGoal [0] *= range + radius;
		vecToGoal [1] *= range + radius;
		//place at starting point
		*io_goalCol = myPlayer->getCol() + vecToGoal [0] + radius;
		*io_goalRow = myPlayer->getRow() + vecToGoal [1] + radius;
	}
}