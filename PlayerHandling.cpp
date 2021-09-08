#include "Playerhandling.hpp"

#include "NetworkCommunication.hpp"
#include "Renderer.hpp"


PlayerHandling::PlayerHandling() {//basically 100% hardcorded stuff for players
	playerCount = 2;
	float vel = 100;//velocity on path if path is given
	int startCol = 700;
	int rectSize = 100;
	int defaultMaxHp = 200;
	int defaultDmg = 10;


	players = new Player * [playerCount];
	for (int i = 0; i < playerCount; i++) {
		int startRow = i * 1000 / playerCount + 200;//move players away from each other in row
		players [i] = new Player(startCol, startRow, rectSize, rectSize, vel, defaultMaxHp, defaultDmg);//places players on map, col dist depends on playercount
	}
}

void PlayerHandling::draw() {
	for (int i = 0; i < playerCount; i++) {
		if (players [i]->getHp() > 0) {
			players [i]->draw();//if he has a path, he moves on this path
		}
	}
}

void PlayerHandling::sendPlayerData() {
	int otherPlayer = 0;
	if (myPlayerI == 0) {
		otherPlayer = 1;
	}

	Player* me = players [myPlayerI];
	if (me->hasPath() == false) {
		NetworkCommunication::addToken(0);//bool if path should be interrupted
		NetworkCommunication::addToken(me->getRow());
		NetworkCommunication::addToken(me->getCol());
	}
	else if (me->hasNewPath == true) {
		me->hasNewPath = false;
		NetworkCommunication::addToken(1);//bool if new bath was found

		NetworkCommunication::addToken(me->pathLenght - me->cPathIndex);//only the path that hasnt been walked yet (lag/connection built up while walking)
		for (int i = me->cPathIndex; i < me->pathLenght; i++) {
			NetworkCommunication::addToken(me->pathXpositions [i]);
			NetworkCommunication::addToken(me->pathYpositions [i]);
		}
	}
	else {
		NetworkCommunication::addToken(2);
	}


	NetworkCommunication::addToken(players [otherPlayer]->getHp());
}

/** Has to pass pathfinding so that we can update pathfinding-graph if player positions changed
**/
int hpSyncDelay = 0;
void PlayerHandling::receivePlayerData(Pathfinding* pathfinding) {
	int otherPlayer = 0;
	if (myPlayerI == 0) {
		otherPlayer = 1;
	}

	int actionIndex = NetworkCommunication::receiveNextToken();

	if (actionIndex == 0) {//interrupt path/no path is there
		if (players [otherPlayer]->hasPath() == true) {
			players [otherPlayer]->deletePath();
		}			
		int row = NetworkCommunication::receiveNextToken();
		int col = NetworkCommunication::receiveNextToken();
		players [otherPlayer]->setRow(row);
		players [otherPlayer]->setCol(col);
	}
	else if (actionIndex == 1) {//new path
		int pathLenght = NetworkCommunication::receiveNextToken();

		int* pathX = new int [pathLenght];
		int* pathY = new int [pathLenght];
		for (int i = 0; i < pathLenght; i++) {
			pathX [i] = NetworkCommunication::receiveNextToken();
			pathY [i] = NetworkCommunication::receiveNextToken();
		}
		players [otherPlayer]->givePath(pathX, pathY, pathLenght);
	}
	else if (actionIndex == 2) {//follow the path given to you
		//do nothing yet
	}
	int hp = NetworkCommunication::receiveNextToken();
	hpSyncDelay ++;
	if (hpSyncDelay > 10) {
		players [myPlayerI]->setHp(hp);
		hpSyncDelay = 0;
	}
}