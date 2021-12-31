#include "Playerhandling.hpp"
#include "NetworkCommunication.hpp"
#include "Renderer.hpp"
#include "GlobalRecources.hpp"
#include "iostream" 
using namespace std;

PlayerHandling::PlayerHandling() {//basically 100% hardcorded stuff for players
	players = shared_ptr<vector<shared_ptr<Player>>>(new vector<shared_ptr<Player>>());
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


	int startY = playerCount * 150 + 200;//move players away from each other in y
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
	int signal = 0;
	if (GlobalRecources::isServer == true) {
		NetworkCommunication::addTokenToAll(players->size());
		for (int cIndex = 0; cIndex < players->size(); cIndex++) {
			const Player* c = players->at(cIndex).get();

			if (c->hasPath() == false) {
				//Option 1: Stop going on path and move to current coordinates of player. SIGNAL -1

				NetworkCommunication::addTokenToAllExceptClient(-1, cIndex);//signal if path should be interrupted
				NetworkCommunication::addTokenToAllExceptClient(c->getY(), cIndex);
				NetworkCommunication::addTokenToAllExceptClient(c->getX(), cIndex);
			}
			else if (c->hasNewPath == true) {
				//Option 2: Transmit new path to clients. SIGNAL -2
				signal = -2;//rn only for debugging purposes

				NetworkCommunication::addTokenToAllExceptClient(-2, cIndex);//signal if new path was found

				GlobalRecources::pfMtx->lock();

				players->at(cIndex)->hasNewPath = false;
				NetworkCommunication::addTokenToAllExceptClient(c->pathLenght - c->cPathIndex, cIndex);//only the path that hasnt been walked yet (lag/connection built up while walking)
				for (int pathIndex = c->cPathIndex; pathIndex < c->pathLenght; pathIndex++) {
					NetworkCommunication::addTokenToAllExceptClient(c->pathXpositions[pathIndex], cIndex);
					NetworkCommunication::addTokenToAllExceptClient(c->pathYpositions[pathIndex], cIndex);
				}
				GlobalRecources::pfMtx->unlock();
			}
			else {
				//Option 3: Do nothing, either stay on path or stay still. SIGNAL -3
				NetworkCommunication::addTokenToAllExceptClient(-3, cIndex);//signal to do nothing
			}
			if (cIndex != 0) {//host doesnt get a message from host (host has playerindex 0)
				NetworkCommunication::addTokenToClient(-3, cIndex);//dont tell current player to do anything with his own path
			}
			NetworkCommunication::addTokenToAll(c->getHp());
		}
	}
	else {
		NetworkCommunication::addTokenToAll(myPlayerI);
		const Player* me = players->at(myPlayerI).get();
		if (me->hasPath() == false) {
			NetworkCommunication::addTokenToAll(-1);//bool if path should be interrupted
			NetworkCommunication::addTokenToAll(me->getY());
			NetworkCommunication::addTokenToAll(me->getX());
		}
		else if (me->hasNewPath == true) {
			NetworkCommunication::addTokenToAll(-2);//bool if new bath was found

			GlobalRecources::pfMtx->lock();

			players->at(myPlayerI)->hasNewPath = false;
			NetworkCommunication::addTokenToAll(me->pathLenght - me->cPathIndex);//only the path that hasnt been walked yet (lag/connection built up while walking)
			for (int pathIndex = me->cPathIndex; pathIndex < me->pathLenght; pathIndex++) {
				NetworkCommunication::addTokenToAll(me->pathXpositions[pathIndex]);
				NetworkCommunication::addTokenToAll(me->pathYpositions[pathIndex]);
			}

			GlobalRecources::pfMtx->unlock();
		}
		else {
			NetworkCommunication::addTokenToAll(-3);
		}
		NetworkCommunication::addTokenToAll(me->getHp());
	}
	NetworkCommunication::addTokenToAll(-11);
}

/** Has to pass pathfinding so that we can update pathfinding-graph if player positions changed
**/
int hpSyncDelay = 0;
void PlayerHandling::receivePlayerData(int clientIndex) {
	if (GlobalRecources::isServer == true) {
		int playerIndex = NetworkCommunication::receiveNextToken(clientIndex);

		int actionIndex = NetworkCommunication::receiveNextToken(clientIndex);
		if (actionIndex == -1) {//interrupt path/no path is there
			if (players->at(playerIndex)->hasPath() == true) {
				players->at(playerIndex)->deletePath();
			}
			int y = NetworkCommunication::receiveNextToken(clientIndex);
			int x = NetworkCommunication::receiveNextToken(clientIndex);
			players->at(playerIndex)->setY(y);
			players->at(playerIndex)->setX(x);
		}
		else if (actionIndex == -2) {//new path
			int pathLenght = NetworkCommunication::receiveNextToken(clientIndex);

			vector<int> pathX(pathLenght);
			vector<int> pathY(pathLenght);
			for (int pathIndex = 0; pathIndex < pathLenght; pathIndex++) {
				pathX[pathIndex] = NetworkCommunication::receiveNextToken(clientIndex);
				pathY[pathIndex] = NetworkCommunication::receiveNextToken(clientIndex);
			}
			players->at(playerIndex)->givePath(move(pathX), move(pathY), pathLenght);
		}
		else if (actionIndex == -3) {//follow the path given to you
			//do nothing yet
		}
		int hp = NetworkCommunication::receiveNextToken(clientIndex);
		hpSyncDelay ++;
		if (hpSyncDelay > 10) {
			players->at(playerIndex)->setHp(hp);
			hpSyncDelay = 0;
		}
	}
	else {
		int playerCount = NetworkCommunication::receiveNextToken(clientIndex);
		//sync player count
		while (playerCount > players->size()) {
			createPlayer();
		}

		for (int playerIndex = 0; playerIndex < playerCount; playerIndex++) {
			int actionIndex = NetworkCommunication::receiveNextToken(clientIndex);
			if (actionIndex == -1) {//interrupt path/no path is there
				if (players->at(playerIndex)->hasPath() == true) {
					players->at(playerIndex)->deletePath();
				}
				int y = NetworkCommunication::receiveNextToken(clientIndex);
				int x = NetworkCommunication::receiveNextToken(clientIndex);
				players->at(playerIndex)->setY(y);
				players->at(playerIndex)->setX(x);
			}
			else if (actionIndex == -2) {//new path
				int pathLenght = NetworkCommunication::receiveNextToken(clientIndex);

				vector<int> pathX(pathLenght);
				vector<int> pathY(pathLenght);
				for (int pathIndex = 0; pathIndex < pathLenght; pathIndex++) {
					pathX[pathIndex] = NetworkCommunication::receiveNextToken(clientIndex);
					pathY[pathIndex] = NetworkCommunication::receiveNextToken(clientIndex);
				}
				players->at(playerIndex)->givePath(move(pathX), move(pathY), pathLenght);
			}
			else if (actionIndex == -3) {//follow the path given to you
				//do nothing yet
			}
			int hp = NetworkCommunication::receiveNextToken(clientIndex);
			hpSyncDelay ++;
			if (hpSyncDelay > 10) {
				players->at(playerIndex)->setHp(hp);
				hpSyncDelay = 0;
			}
		}
	}
	int checkUp = NetworkCommunication::receiveNextToken(clientIndex);
	if (checkUp != -11) {
		std::cout << "Error, expected -11 as next token in PlayerHandling receive but received " << checkUp;
		std::cout << "\nfull data: " << NetworkCommunication::getReceivedData(1);
		exit(0);
	}
}