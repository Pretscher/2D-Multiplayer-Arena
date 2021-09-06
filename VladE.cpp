#include "VladE.hpp"
#include <chrono>
#include "Renderer.hpp"
#include "GlobalRecources.hpp"
static int i_onCDPhase = 0;
static int i_addToNetworkPhase = 0;
static int i_abilityIndex = 2;

VladE::VladE(int i_myPlayerIndex) : Ability(i_myPlayerIndex, false, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {
	endPhaseAfterMS(1500);
}
//constructor through networking
VladE::VladE(int i_myPlayerIndex, int i_currentCol, int i_phase, int i_timeInPhase) : Ability(i_myPlayerIndex, true, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {
	endPhaseAfterMS(1500);
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
	projectileCount = 8;
	projectiles = new Projectile* [projectileCount];
	
	for (int i = 0; i < projectileCount; i++) {
		int startRow;
		int startCol;

		Player* myPlayer = GlobalRecources::players [myPlayerIndex];
		int pCol = myPlayer->getCol();
		int pRow = myPlayer->getRow();
		int pWidth = myPlayer->getWidth();
		int pHeight = myPlayer->getHeight();

		switch (i) {
		case 0:
			startRow = pRow;
			startCol = pCol;
			break;
		case 1:
			startRow = pRow + pHeight;
			startCol = pCol;
			break;
		case 2:
			startRow = pRow;
			startCol = pCol + pWidth;
			break;
		case 3:
			startRow = pRow + pHeight;
			startCol = pCol + pWidth;
			break;
		case 4:
			startRow = pRow + (pHeight / 2);
			startCol = pCol;
			break;
		case 5:
			startRow = pRow;
			startCol = pCol + (pWidth / 2);
			break;
		case 6:
			startRow = pRow + pHeight;
			startCol = pCol + (pWidth / 2);
			break;
		case 7:
			startRow = pRow + (pHeight / 2);
			startCol = pCol + pWidth;
			break;
		default:
			break;
		}

		int goalRow = 1000, goalCol = 1000;
		projectiles [i] = new Projectile(startRow, startCol, velocity, goalRow, goalCol, false, radius, myPlayer);
	}
}

void VladE::limitPosToRange(int* io_goalRow, int* io_goalCol) {
	Player* myPlayer = GlobalRecources::players [myPlayerIndex];
	
	float* vecToGoal = new float [2];
	vecToGoal [0] = *io_goalCol - myPlayer->getCol();
	vecToGoal [1] = *io_goalRow - myPlayer->getRow();
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
		*io_goalCol = myPlayer->getCol() + vecToGoal [0];
		*io_goalRow = myPlayer->getRow() + vecToGoal [1];
	}
}
void VladE::execute1() {
	auto terrain = GlobalRecources::terrain->getCollidables();
	for (int i = 0; i < projectileCount; i++) {
		projectiles[i]->move(GlobalRecources::worldRows, GlobalRecources::worldCols, terrain->data(), terrain->size());
	}


	//end ability after all projectiles reached their destination
	bool someoneAlive = false;
	for (int i = 0; i < projectileCount; i++) {
		if (projectiles [i]->getDead() == false) {
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
		projectiles [i]->draw(sf::Color(150, 0, 0, 255));
	}
}

void VladE::init2() {
	finished = true;
}

void VladE::execute2() {

}

void VladE::draw2() {

}