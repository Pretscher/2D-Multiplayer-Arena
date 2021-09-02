#include "Playerhandling.hpp"

#include "NetworkCommunication.hpp"
#include "Renderer.hpp"

int otherPlayer = 0;
PlayerHandling::PlayerHandling() {//basically 100% hardcorded stuff for players
	playerCount = 2;
	float vel = 50;//velocity on path if path is given
	int startCol = 700;
	int rectSize = 100;
	int defaultMaxHp = 200;
	int defaultDmg = 10;

	if (myPlayerI == 0) {
		otherPlayer = 1;
	}

	players = new Player * [playerCount];
	for (int i = 0; i < playerCount; i++) {
		int startRow = i * 1000 / playerCount + 200;//move players away from each other in row
		players[i] = new Player(startCol, startRow, rectSize, rectSize, vel, defaultMaxHp, defaultDmg);//places players on map, col dist depends on playercount
	}
}

void PlayerHandling::draw() {
	for (int i = 0; i < playerCount; i++) {
		if (players[i]->getHp() > 0) {
			players[i]->draw();//if he has a path, he moves on this path
		}
	}
}

void PlayerHandling::sendPlayerData() {
	Player* me = players [myPlayerI];
	if (me->hasNewPath == true) {
		me->hasNewPath = false;
		NetworkCommunication::addToken(1);//bool if new bath was found

		NetworkCommunication::addToken(me->pathLenght - me->cPathIndex);//only the path that hasnt been walked yet (lag/connection built up while walking)
		for (int i = me->cPathIndex; i < me->pathLenght; i++) {
			NetworkCommunication::addToken(me->pathXpositions [i]);
			NetworkCommunication::addToken(me->pathYpositions [i]);
		}
	}
	else {
		NetworkCommunication::addToken(0);
	}


	NetworkCommunication::addToken(players[otherPlayer]->getHp());
}

/** Has to pass pathfinding so that we can update pathfinding-graph if player positions changed
**/
int hpSyncDelay = 0;
void PlayerHandling::receivePlayerData(Pathfinding* pathfinding) {
	if (NetworkCommunication::receiveNextToken() == 1) {//new path
		int pathLenght = NetworkCommunication::receiveNextToken();
		
		int* pathX = new int [pathLenght];
		int* pathY = new int [pathLenght];
		for (int i = 0; i < pathLenght; i++) {
			pathX [i] = NetworkCommunication::receiveNextToken();
			pathY [i] = NetworkCommunication::receiveNextToken();
		}
		players [otherPlayer]->givePath(pathX, pathY, pathLenght);
	}

	int hp = NetworkCommunication::receiveNextToken();
	hpSyncDelay ++;
	if(hpSyncDelay > 10) {
		players[myPlayerI]->setHp(hp);
		hpSyncDelay = 0;
	}
}