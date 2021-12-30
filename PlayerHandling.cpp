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
	if (GlobalRecources::isServer == true) {
		NetworkCommunication::addTokenToAll(players->size());
		for (int i = 0; i < players->size(); i++) {
			const Player* c = players->at(i).get();
			string s1 = NetworkCommunication::getSentData(i);
			if (c->hasPath() == false) {
				NetworkCommunication::addTokenToAllExceptClient(0, i);//bool if path should be interrupted
				NetworkCommunication::addTokenToAllExceptClient(c->getY(), i);
				NetworkCommunication::addTokenToAllExceptClient(c->getX(), i);
			}
			else if (c->hasNewPath == true) {
				players->at(i)->hasNewPath = false;
				NetworkCommunication::addTokenToAllExceptClient(1, i);//bool if new bath was found

				NetworkCommunication::addTokenToAllExceptClient(c->pathLenght - c->cPathIndex - 1, i);//only the path that hasnt been walked yet (lag/connection built up while walking)
				for (int i = c->cPathIndex; i < c->pathLenght; i++) {
					NetworkCommunication::addTokenToAllExceptClient(c->pathXpositions[i], i);
					NetworkCommunication::addTokenToAllExceptClient(c->pathYpositions[i], i);
				}
			}
			else {
				NetworkCommunication::addTokenToAllExceptClient(2, i);
			}
			if (i != 0) {//host doesnt get a message from host (host has playerindex 0)
				NetworkCommunication::addTokenToClient(2, i);//dont tell current player to do anything with his own path
			}
			NetworkCommunication::addTokenToAll(c->getHp());
			string s2 = NetworkCommunication::getSentData(i);
		}
	}
	else {
		NetworkCommunication::addTokenToAll(myPlayerI);
		const Player* me = players->at(myPlayerI).get();
		if (me->hasPath() == false) {
			NetworkCommunication::addTokenToAll(0);//bool if path should be interrupted
			NetworkCommunication::addTokenToAll(me->getY());
			NetworkCommunication::addTokenToAll(me->getX());
		}
		else if (me->hasNewPath == true) {
			players->at(myPlayerI)->hasNewPath = false;
			NetworkCommunication::addTokenToAll(1);//bool if new bath was found

			NetworkCommunication::addTokenToAll(me->pathLenght - me->cPathIndex - 1);//only the path that hasnt been walked yet (lag/connection built up while walking)
			for (int i = me->cPathIndex; i < me->pathLenght; i++) {
				int x = me->pathXpositions[i];
				NetworkCommunication::addTokenToAll(me->pathXpositions[i]);
				NetworkCommunication::addTokenToAll(me->pathYpositions[i]);
			}
		}
		else {
			NetworkCommunication::addTokenToAll(2);
		}
		NetworkCommunication::addTokenToAll(me->getHp());
	}
}

/** Has to pass pathfinding so that we can update pathfinding-graph if player positions changed
**/
int hpSyncDelay = 0;
void PlayerHandling::receivePlayerData(int clientIndex) {
	if (GlobalRecources::isServer == true) {
		int playerIndex = NetworkCommunication::receiveNextToken(clientIndex);

		int actionIndex = NetworkCommunication::receiveNextToken(clientIndex);
		if (actionIndex == 0) {//interrupt path/no path is there
			if (players->at(playerIndex)->hasPath() == true) {
				players->at(playerIndex)->deletePath();
			}
			int y = NetworkCommunication::receiveNextToken(clientIndex);
			int x = NetworkCommunication::receiveNextToken(clientIndex);
			players->at(playerIndex)->setY(y);
			players->at(playerIndex)->setX(x);
		}
		else if (actionIndex == 1) {//new path
			int pathLenght = NetworkCommunication::receiveNextToken(clientIndex);

			vector<int> pathX(pathLenght);
			vector<int> pathY(pathLenght);
			for (int i = 0; i < pathLenght; i++) {
				pathX[i] = NetworkCommunication::receiveNextToken(clientIndex);
				pathY[i] = NetworkCommunication::receiveNextToken(clientIndex);
			}
			players->at(playerIndex)->givePath(move(pathX), move(pathY), pathLenght);
		}
		else if (actionIndex == 2) {//follow the path given to you
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
		vector<int> v1 = NetworkCommunication::getReceivedData(clientIndex);
		for (int i = 0; i < playerCount; i++) {
			int actionIndex = NetworkCommunication::receiveNextToken(clientIndex);
			if (actionIndex == 0) {//interrupt path/no path is there
				if (players->at(i)->hasPath() == true) {
					players->at(i)->deletePath();
				}
				int y = NetworkCommunication::receiveNextToken(clientIndex);
				int x = NetworkCommunication::receiveNextToken(clientIndex);
				players->at(i)->setY(y);
				players->at(i)->setX(x);
			}
			else if (actionIndex == 1) {//new path
				int pathLenght = NetworkCommunication::receiveNextToken(clientIndex);

				vector<int> pathX(pathLenght);
				vector<int> pathY(pathLenght);
				for (int i = 0; i < pathLenght; i++) {
					pathX[i] = NetworkCommunication::receiveNextToken(clientIndex);
					pathY[i] = NetworkCommunication::receiveNextToken(clientIndex);
				}
				players->at(i)->givePath(move(pathX), move(pathY), pathLenght);
			}
			else if (actionIndex == 2) {//follow the path given to you
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
}