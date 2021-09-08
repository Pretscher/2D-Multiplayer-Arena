#pragma once
#include "VladR.hpp"
#include "GlobalRecources.hpp"
#include "Utils.hpp"


static int i_onCDPhase = 2;
static int i_addToNetworkPhase = 2;
static int i_abilityIndex = 3;

VladR::VladR(int i_myPlayerIndex) : Ability(i_myPlayerIndex, false, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {
	indicator = new AOEonRangeIndicator(i_myPlayerIndex, range, radius);
}
//constructor through networking
VladR::VladR(int i_myPlayerIndex, int i_phase, int i_timeInPhase) : Ability(i_myPlayerIndex, true, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {
	for (int i = 0; i < i_phase; i++) {
		nextPhase();
	}

	if (i_phase == 2) {
		networkingTimeInPhase = i_timeInPhase;
	}
}

void VladR::execute0() {
	indicator->update();
	if (indicator->isDestSelected() == true) {
		row = indicator->getDestRow();
		col = indicator->getDestCol();
		nextPhase();
	}
	if (indicator->endWithoutAction() == true) {
		finished = true;
	}
}

void VladR::draw0() {
	indicator->draw();
}

void VladR::init1() {
	Player* me = GlobalRecources::players[myPlayerIndex];
	//if player out of range, run into range
	int halfW = me->getWidth() / 2;//we need this to calc the range between the player's coord centers
	int halfH = me->getHeight() / 2;
	float dist = Utils::calcDist2D(me->getCol() + halfW, col + halfW, me->getRow() + halfH, row + halfH);
	if (dist > range + radius) {//if player is too far away
		GlobalRecources::pFinding->findPath(col, row, myPlayerIndex); //find a path to him
		abilityPathIndex = GlobalRecources::players[myPlayerIndex]->pathsFound;
	}
	else {
		nextPhase();//if already in range, just start casting without moving
	}
}

void VladR::execute1() {


	//multithreading problem: we want, if another path is found for the player (e.g. through clicking)
	//to stop going on the path the ability wants to find. But because of multithreading we cant say
	//when the pathfinding is finished with this particular path, so we just count the paths up in the
	//player obj and if the index is equal to abiltyPathIndex its still finding the path (same index
	//as when it was saved) and if its one higher the path was found. if its 2 higher a new path
	//was found and we interrupt.
	bool stop = false;
	if (GlobalRecources::players[myPlayerIndex]->pathsFound > abilityPathIndex + 1) {
		stop = true;
	}
	if (stop == false) {
		Player* me = GlobalRecources::players[myPlayerIndex];
		int halfW = me->getWidth() / 2;
		int halfH = me->getHeight() / 2;
		if (Utils::calcDist2D(me->getCol() + halfW, col, me->getRow() + halfH, row) < range + radius) {
			//got into range, stop going on path an cast ability
			GlobalRecources::players[myPlayerIndex]->deletePath();
			nextPhase();
		}
	}
	else {
		//clicked somewhere else while finding path to target player to get in range => abort cast
		finished = true;
	}
}

void VladR::draw1() {
	
}

void VladR::init2() {
	affectedPlayers = new Player*[GlobalRecources::playerCount];//just init static array to max player count, theyre few

	//save players who are still in range when this phase is activated as affected, damagin them after 4 seconds
	for (int i = 0; i < GlobalRecources::playerCount; i++) {
		if (i != myPlayerIndex) {
			Player* cPlayer = GlobalRecources::players[i];
			if (cPlayer->targetAble == true) {
				if (Utils::calcDist2D(col, cPlayer->getCol(), row, cPlayer->getRow()) < radius) {
					affectedPlayers[affectedPlayerCount] = cPlayer;
					affectedPlayerCount ++;
				}
			}
		}
	}

	endPhaseAfterMS(timeTillProc - networkingTimeInPhase);
}

void VladR::execute2() {
	
}

void VladR::draw2() {
	for (int i = 0; i < affectedPlayerCount; i++) {
		Player* cPlayer = affectedPlayers[i];
		Renderer::drawCircle(cPlayer->getRow(), cPlayer->getCol(), 50, sf::Color(150, 0, 0, 255), false, 20, false);
	}
	float phaseFinishedPercent = (float)this->getTimeSincePhaseStart(this->getPhase()) / timeTillProc;
	if (phaseFinishedPercent < 0.25f) {


		float alpha = 255 * abs(1 - phaseFinishedPercent * 4);//fade out with time
		Renderer::drawCircle(row - radius, col - radius, radius, sf::Color(100, 0, 0, alpha), true, 0, false);

		Renderer::drawCircle(row - radius, col - radius, radius, sf::Color(200, 0, 0, alpha), false, 30, false);
		int secondRadiusOffset = 50;
		Renderer::drawCircle(row - radius + secondRadiusOffset, col - radius + secondRadiusOffset, radius - secondRadiusOffset, 
												sf::Color(150, 0, 0, alpha), false, 50, false);
	}
}


void VladR::init3() {
	//deal damage to affected players
	for (int i = 0; i < affectedPlayerCount; i++) {
		Player* cPlayer = affectedPlayers[i];
		cPlayer->setHp(cPlayer->getHp() - damage);
	}

	//create projectile that flies to player and heals him
	Player* me = GlobalRecources::players[myPlayerIndex];
	int halfW = me->getWidth() / 2;
	bloodBall = new Projectile(row, col, flyBackVelocity, me->getRow() + halfW, me->getCol() + halfW, false, flyBackRadius, me);

	lastRows = new int[positionsSavedCount];
	lastCols = new int[positionsSavedCount];
	for (int i = 0; i < positionsSavedCount; i++) {
		lastRows[i] = -1;
		lastCols[i] = -1;
	}
	tempFlybackRow = me->getRow() + halfW;
	tempFlybackCol = me->getCol() + halfW;
}

void VladR::execute3() {
	checkBloodballCollision();
	findNewPathToPlayerTimer ++;
	if (findNewPathToPlayerTimer % 10 == true) {
		followPlayer();
	}

	lastRows[cPositionSaveIndex] = bloodBall->getRow();
	lastCols[cPositionSaveIndex] = bloodBall->getCol();
	cPositionSaveIndex ++;
	if (cPositionSaveIndex >= positionsSavedCount) {
		cPositionSaveIndex = 0;
	}

	bloodBall->move(GlobalRecources::worldRows, GlobalRecources::worldCols, nullptr, 0);//should go through walls so we just dont pass them
}


void VladR::draw3() {
	bloodBall->draw(sf::Color(150, 0, 0, 255));
	for (int i = 0; i < positionsSavedCount; i++) {
		if (lastRows[i] != -1) {
			Renderer::drawCircle(lastRows[i], lastCols[i], bloodBall->getRadius(), sf::Color(150, 0, 0, 255), true, 0, false);
		}
	}
}


void VladR::checkBloodballCollision() {
	Player* me = GlobalRecources::players[myPlayerIndex];
	//blood ball got to enemy and should fly back
	if (Utils::collisionRectCircle(me->getRow(), me->getCol(), me->getWidth(), me->getHeight(),
		bloodBall->getRow(), bloodBall->getCol(), bloodBall->getRadius(), 10) == true) {
		
		int heal = + (healPerPlayer * affectedPlayerCount);
		if (me->getHp() + heal <= me->getMaxHp()) {
			me->setHp(me->getHp() + heal);
		}
		else {
			me->setHp(me->getMaxHp());
		}
		finished = true;
	}
}

void VladR::followPlayer() {
	Player* me = GlobalRecources::players[myPlayerIndex];
	if (tempFlybackRow != me->getRow() || tempFlybackCol != me->getCol()) {
		tempFlybackRow = me->getRow();
		tempFlybackCol = me->getCol();
		int tempBBrow = bloodBall->getRow();
		int tempBBcol = bloodBall->getCol();


		int halfW = me->getWidth() / 2;
		delete bloodBall;//definitly exists at this point so we can delete it
		bloodBall = new Projectile(tempBBrow + flyBackRadius, tempBBcol + flyBackRadius, flyBackVelocity,
			tempFlybackRow + halfW, tempFlybackCol + halfW, false, flyBackRadius, me);
	}
}