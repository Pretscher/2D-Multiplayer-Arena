#include "Playerhandling.hpp"
#include "NetworkCommunication.hpp"
#include "Renderer.hpp"
#include "GlobalRecources.hpp"
#include "iostream" 
using namespace std;

PlayerHandling::PlayerHandling() {//basically 100% hardcorded stuff for players
	createPlayer();//Create hosting player
	GlobalRecources::players = players;
	GlobalRecources::playerCount = playerCount;
}

void PlayerHandling::createPlayer() {
	float vel = 50;//velocity on path if path is given
	int startX = 700;
	int rectSize = 100;
	int defaultMaxHp = 500;
	int defaultDmg = 10;


	players = shared_ptr<vector<shared_ptr<Player>>>(new vector<shared_ptr<Player>>(playerCount));
	int startY = playerCount * 1000 + 200;//move players away from each other in y
	players->push_back(shared_ptr<Player>(new Player(startX, startY, rectSize, rectSize, vel, defaultMaxHp, defaultDmg)));//places players on map, x dist depends on playercount

	playerCount = players->size();
}

void PlayerHandling::draw() {
	for (int i = 0; i < playerCount; i++) {
		if (players->at(i)->getHp() > 0) {
			players->at(i)->draw();//if he has a path, he moves on this path
		}
	}
}

void PlayerHandling::sendPlayerData() {
	int otherPlayer = 0;
	if (myPlayerI == 0) {
		otherPlayer = 1;
	}

	const Player* me = players->at(myPlayerI).get();
	if (me->hasPath() == false) {
		NetworkCommunication::addToken(0);//bool if path should be interrupted
		NetworkCommunication::addToken(me->getY());
		NetworkCommunication::addToken(me->getX());
	}
	else if (me->hasNewPath == true) {
		players->at(myPlayerI)->hasNewPath = false;
		NetworkCommunication::addToken(1);//bool if new bath was found

		NetworkCommunication::addToken(me->pathLenght - me->cPathIndex);//only the path that hasnt been walked yet (lag/connection built up while walking)
		for (int i = me->cPathIndex; i < me->pathLenght; i++) {
			int x = me->pathXpositions[i];
			NetworkCommunication::addToken(me->pathXpositions[i]);
			NetworkCommunication::addToken(me->pathYpositions[i]);
		}
	}
	else {
		NetworkCommunication::addToken(2);
	}


	NetworkCommunication::addToken(players->at(otherPlayer)->getHp());
}

/** Has to pass pathfinding so that we can update pathfinding-graph if player positions changed
**/
int hpSyncDelay = 0;
void PlayerHandling::receivePlayerData(int index) {

	int actionIndex = NetworkCommunication::receiveNextToken(index);

	if (actionIndex == 0) {//interrupt path/no path is there
		if (players->at(index)->hasPath() == true) {
			players->at(index)->deletePath();
		}			
		int y = NetworkCommunication::receiveNextToken(index);
		int x = NetworkCommunication::receiveNextToken(index);
		players->at(index)->setY(y);
		players->at(index)->setX(x);
	}
	else if (actionIndex == 1) {//new path
		int pathLenght = NetworkCommunication::receiveNextToken(index);

		vector<int> pathX(pathLenght);
		vector<int> pathY(pathLenght);
		for (int i = 0; i < pathLenght; i++) {
			pathX[i] = NetworkCommunication::receiveNextToken(index);
			pathY[i] = NetworkCommunication::receiveNextToken(index);
		}
		players->at(index)->givePath(move(pathX), move(pathY), pathLenght);
	}
	else if (actionIndex == 2) {//follow the path given to you
		//do nothing yet
	}
	int hp = NetworkCommunication::receiveNextToken(index);
	hpSyncDelay ++;
	if (hpSyncDelay > 10) {
		players->at(myPlayerI)->setHp(hp);
		hpSyncDelay = 0;
	}
}