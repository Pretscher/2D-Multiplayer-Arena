#pragma once
#include "VladR.hpp"
#include "GlobalRecources.hpp"
#include "Utils.hpp"
#include <iostream>

static int i_onCDPhase = 2;
static int i_addToNetworkPhase = 2;
static int i_abilityIndex = 3;

VladR::VladR(int i_myPlayerIndex) : Ability(i_myPlayerIndex, false, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {
	indicator = new AOEonRangeIndicator(i_myPlayerIndex, range, radius);
}
//constructor through networking
VladR::VladR() : Ability(NetworkCommunication::receiveNextToken(), true, i_onCDPhase, i_addToNetworkPhase, i_abilityIndex) {
	this->networkingTimeInPhase = NetworkCommunication::receiveNextToken();
	this->y = NetworkCommunication::receiveNextToken();
	this->x = NetworkCommunication::receiveNextToken();
	for (int i = 0; i < 1; i++) {
		nextPhase();
	}
}

void VladR::execute0() {
	indicator->update();
	if (indicator->isDestSelected() == true) {
		y = indicator->getDestY();
		x = indicator->getDestX();
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
	const Player* me = GlobalRecources::players->at(myPlayerIndex).get();
	//if player out of range, run into range
	int halfW = me->getWidth() / 2;//we need this to calc the range between the player's coord centers
	int halfH = me->getHeight() / 2;
	float dist = Utils::calcDist2D(me->getX() + halfW, x + halfW, me->getY() + halfH, y + halfH);
	if (dist > range + radius) {//if player is too far away
		GlobalRecources::pFinding->findPath(x, y, myPlayerIndex); //find a path to him
		abilityPathIndex = GlobalRecources::players->at(myPlayerIndex)->pathsFound;
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
	if (GlobalRecources::players->at(myPlayerIndex)->pathsFound > abilityPathIndex + 1) {
		stop = true;
	}
	if (stop == false) {
		const Player* me = GlobalRecources::players->at(myPlayerIndex).get();
		int halfW = me->getWidth() / 2;
		int halfH = me->getHeight() / 2;
		if (Utils::calcDist2D(me->getX() + halfW, x, me->getY() + halfH, y) < range + radius) {
			//got into range, stop going on path an cast ability
			GlobalRecources::players->at(myPlayerIndex)->deletePath();
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
	affectedPlayers = shared_ptr<shared_ptr<Player>[]>(new shared_ptr<Player>[GlobalRecources::playerCount]);//just init static array to max player count, theyre few

	//save players who are still in range when this phase is activated as affected, damagin them after 4 seconds
	for (int i = 0; i < GlobalRecources::playerCount; i++) {
		if (i != myPlayerIndex) {
			shared_ptr<Player> cPlayer = GlobalRecources::players->at(i);
			if (cPlayer->targetAble == true) {
				if (Utils::calcDist2D(x, cPlayer->getX(), y, cPlayer->getY()) < radius) {
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
		const Player* cPlayer = affectedPlayers[i].get();
		Renderer::drawCircle(cPlayer->getX(), cPlayer->getY(), 50, sf::Color(150, 0, 0, 255), false, 20, false);
	}
	float phaseFinishedPercent = (float)this->getTimeSincePhaseStart(this->getPhase()) / timeTillProc;
	if (phaseFinishedPercent < 0.25f) {


		float alpha = 255 * abs(1 - phaseFinishedPercent * 4);//fade out with time
		Renderer::drawCircle(x - radius, y - radius, radius, sf::Color(100, 0, 0, alpha), true, 0, false);

		Renderer::drawCircle(x - radius, y - radius, radius, sf::Color(200, 0, 0, alpha), false, 30, false);
		int secondRadiusOffset = 50;
		Renderer::drawCircle(x - radius + secondRadiusOffset, y - radius + secondRadiusOffset, radius - secondRadiusOffset, 
												sf::Color(150, 0, 0, alpha), false, 50, false);
	}
}


void VladR::init3() {
	//deal damage to affected players
	for (int i = 0; i < affectedPlayerCount; i++) {
		shared_ptr<Player> cPlayer = affectedPlayers[i];
		cPlayer->setHp(cPlayer->getHp() - damage);
	}

	//create projectile that flies to player and heals him
	const Player* me = GlobalRecources::players->at(myPlayerIndex).get();
	int halfW = me->getWidth() / 2;
	bloodBall = new Projectile(y, x, flyBackVelocity, me->getY() + halfW, me->getX() + halfW, false, flyBackRadius, GlobalRecources::players->at(myPlayerIndex));

	lastYs = new int[positionsSavedCount];
	lastXs = new int[positionsSavedCount];
	for (int i = 0; i < positionsSavedCount; i++) {
		lastYs[i] = -1;
		lastXs[i] = -1;
	}
	tempFlybackY = me->getY() + halfW;
	tempFlybackX = me->getX() + halfW;
}

void VladR::execute3() {
	checkBloodballCollision();
	findNewPathToPlayerTimer ++;
	if (findNewPathToPlayerTimer % 10 == true) {
		followPlayer();
	}

	lastYs[cPositionSaveIndex] = bloodBall->getY();
	lastXs[cPositionSaveIndex] = bloodBall->getX();
	cPositionSaveIndex ++;
	if (cPositionSaveIndex >= positionsSavedCount) {
		cPositionSaveIndex = 0;
	}

	bloodBall->move(GlobalRecources::worldHeight, GlobalRecources::worldWidth, nullptr, 0);//should go through walls so we just dont pass them
}


void VladR::draw3() {
	bloodBall->draw(sf::Color(150, 0, 0, 255));
	for (int i = 0; i < positionsSavedCount; i++) {
		if (lastYs[i] != -1) {
			Renderer::drawCircle(lastXs[i], lastYs[i], bloodBall->getRadius(), sf::Color(150, 0, 0, 255), true, 0, false);
		}
	}
}


void VladR::checkBloodballCollision() {
	shared_ptr<Player> me = GlobalRecources::players->at(myPlayerIndex);
	//blood ball got to enemy and should fly back
	if (Utils::colisionRectCircle(me->getY(), me->getX(), me->getWidth(), me->getHeight(),
		bloodBall->getY(), bloodBall->getX(), bloodBall->getRadius(), 10) == true) {
		
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
	shared_ptr<Player> me = GlobalRecources::players->at(myPlayerIndex);
	if (tempFlybackY != me->getY() || tempFlybackX != me->getX()) {
		tempFlybackY = me->getY();
		tempFlybackX = me->getX();
		int tempBBy = bloodBall->getY();
		int tempBBx = bloodBall->getX();


		int halfW = me->getWidth() / 2;
		delete bloodBall;//definitly exists at this point so we can delete it
		bloodBall = new Projectile(tempBBy + flyBackRadius, tempBBx + flyBackRadius, flyBackVelocity,
			tempFlybackY + halfW, tempFlybackX + halfW, false, flyBackRadius, me);
	}
}

void VladR::send() {
	NetworkCommunication::addToken(this->myPlayerIndex);
	NetworkCommunication::addToken(getTimeSincePhaseStart(2));
	NetworkCommunication::addToken(this->y);
	NetworkCommunication::addToken(this->x);
}