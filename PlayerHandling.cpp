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
				//so that the current index and path lenght don't change in another thread while sending
				int fixedIndex = c->cPathIndex;
				int fixedLenght = c->pathLenght;
				NetworkCommunication::addTokenToAllExceptClient(fixedLenght - fixedIndex, cIndex);//only the path that hasnt been walked yet (lag/connection built up while walking)
				
				for (int pathIndex = fixedIndex; pathIndex < fixedLenght; pathIndex++) {
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
			/*string s2 = NetworkCommunication::getSentData(i);
			if (signal == -2) {
				std::cout << s2;
			}*/
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
			players->at(myPlayerI)->hasNewPath = false;
			NetworkCommunication::addTokenToAll(-2);//bool if new bath was found

			int fixedIndex = me->cPathIndex;//so that the index doesnt change in another thread (NOT SAFE DO CHANGE LATER)
			NetworkCommunication::addTokenToAll(me->pathLenght - fixedIndex);//only the path that hasnt been walked yet (lag/connection built up while walking)
			for (int j = fixedIndex; j < me->pathLenght; j++) {
				NetworkCommunication::addTokenToAll(me->pathXpositions[j]);
				NetworkCommunication::addTokenToAll(me->pathYpositions[j]);
			}
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
			for (int j = 0; j < pathLenght; j++) {
				pathX[j] = NetworkCommunication::receiveNextToken(clientIndex);
				pathY[j] = NetworkCommunication::receiveNextToken(clientIndex);
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

		for (int i = 0; i < playerCount; i++) {
			int actionIndex = NetworkCommunication::receiveNextToken(clientIndex);
			if (actionIndex == -1) {//interrupt path/no path is there
				if (players->at(i)->hasPath() == true) {
					players->at(i)->deletePath();
				}
				int y = NetworkCommunication::receiveNextToken(clientIndex);
				int x = NetworkCommunication::receiveNextToken(clientIndex);
				players->at(i)->setY(y);
				players->at(i)->setX(x);
			}
			else if (actionIndex == -2) {//new path
				int pathLenght = NetworkCommunication::receiveNextToken(clientIndex);

				vector<int> pathX(pathLenght);
				vector<int> pathY(pathLenght);
				for (int j = 0; j < pathLenght; j++) {
					pathX[j] = NetworkCommunication::receiveNextToken(clientIndex);
					pathY[j] = NetworkCommunication::receiveNextToken(clientIndex);
				}
				players->at(i)->givePath(move(pathX), move(pathY), pathLenght);
			}
			else if (actionIndex == -3) {//follow the path given to you
				//do nothing yet
			}
			int hp = NetworkCommunication::receiveNextToken(clientIndex);
			hpSyncDelay ++;
			if (hpSyncDelay > 10) {
				players->at(i)->setHp(hp);
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