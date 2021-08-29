#include "Playerhandling.hpp"

#include "NetworkCommunication.hpp"
#include "Renderer.hpp"

PlayerHandling::PlayerHandling() {//basically 100% hardcorded stuff for players
	playerCount = 2;
	float vel = 50;//velocity on path if path is given
	int startCol = 700;
	int rectSize = 100;
	int defaultMaxHp = 200;
	int defaultDmg = 10;

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
	NetworkCommunication::addToken(players[myPlayerI]->getRow());
	NetworkCommunication::addToken(players[myPlayerI]->getCol());
	NetworkCommunication::addToken(players[myPlayerI]->getTextureIndex());
	
	int otherPlayer = 0;
	if (myPlayerI == 0) {
		otherPlayer = 1;
	}
	NetworkCommunication::addToken(players[otherPlayer]->getHp());
}

/** Has to pass pathfinding so that we can update pathfinding-graph if player positions changed
**/
int hpSyncDelay = 0;
void PlayerHandling::receivePlayerData(Pathfinding* pathfinding) {
	int otherPlayer = 0;
	if (myPlayerI == 0) {
		otherPlayer = 1;
	}

	int tempRow = players[otherPlayer]->getRow();
	int tempCol = players[otherPlayer]->getCol();

	players[otherPlayer]->setRow(NetworkCommunication::receiveNextToken());
	players[otherPlayer]->setCol(NetworkCommunication::receiveNextToken());
	players[otherPlayer]->setTexture(NetworkCommunication::receiveNextToken());
	players[myPlayerI]->setHp(NetworkCommunication::receiveNextToken());

	pathfinding->enableArea(tempRow, tempCol, players[0]->getWidth() + 100, players[0]->getHeight() + 100);//enable old position
	pathfinding->disableArea(players[otherPlayer]->getRow(), players[otherPlayer]->getCol(),
		players[0]->getWidth(), players[0]->getHeight());//disable new position
	//could have enabled other players position aswell
	pathfinding->disableArea(players[myPlayerI]->getRow(), players[myPlayerI]->getCol(), players[0]->getWidth(), players[0]->getHeight());//disable new position

}