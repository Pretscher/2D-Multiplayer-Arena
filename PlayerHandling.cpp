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
}

void PlayerHandling::createPlayer() {
	float vel = 50;//velocity on path if path is given
	int startX = 700;
	int rectSize = 100;
	int defaultMaxHp = 500;
	int defaultDmg = 10;


	int startY = players->size() * 150 + 200;//move players away from each other in y
	players->push_back(shared_ptr<Player>(new Player(startX, startY, rectSize, rectSize, vel, defaultMaxHp, defaultDmg)));//places players on map, x dist depends on playercount
}

void PlayerHandling::draw() {
	for (int i = 0; i < players->size(); i++) {
		if (players->at(i)->getHp() > 0) {
			players->at(i)->draw();//if he has a path, he moves on this path
		}
	}
}

void PlayerHandling::sendPlayerData() {

	GlobalRecources::pfMtx->lock();//there is no way to do this more efficiently and safe than blocking for the entire transmission
	if (GlobalRecources::isServer == true) {
		NetworkCommunication::addTokenToAll(players->size());
		for (int clientIndex = 0; clientIndex < players->size(); clientIndex++) {
			const Player* cPlayer = players->at(clientIndex).get();

			if (clientIndex > 0 && GlobalRecources::initNetwork->at(clientIndex - 1) == false) {
				NetworkCommunication::addTokenToClient(-4, clientIndex);//signal that player data will be transmitted
				players->at(clientIndex)->interruptedPath = false;//only once per interrupt

				//transmit current coordinates-----------------------------------------------------------------------------------------------------
				for (int cPlayer = 0; cPlayer < players->size(); cPlayer++) {
					NetworkCommunication::addTokenToClient(players->at(cPlayer)->getY(), clientIndex);
					NetworkCommunication::addTokenToClient(players->at(cPlayer)->getX(), clientIndex);
				}
				//transmit paths and indices in those paths that the players might be travelling while connection happens--------------------------
				for (int playerI = 0; playerI < players->size(); playerI++) {
					const Player* player = players->at(playerI).get();
					NetworkCommunication::addTokenToClient(-5, clientIndex);//signal that player has no path
					if (player->hasPath() == true) {
						NetworkCommunication::addTokenToClient(-6, clientIndex);//signal that player has path which will be transmitted
						NetworkCommunication::addTokenToClient(player->pathLenght - player->cPathIndex, clientIndex);//only the path that hasnt been walked yet (lag/connection built up while walking)
						for (int pathIndex = player->cPathIndex; pathIndex < player->pathLenght; pathIndex++) {
							NetworkCommunication::addTokenToClient(player->pathXpositions[pathIndex], clientIndex);
							NetworkCommunication::addTokenToClient(player->pathYpositions[pathIndex], clientIndex);
						}
					}
				}
			}
			else if (cPlayer->interruptedPath == true) {//on first send, sync coordinates
				//Option 1: Stop going on path and move to current coordinates of player. SIGNAL -1
				NetworkCommunication::addTokenToAllExceptClient(-1, clientIndex);//signal if path should be interrupted

				//we use the smart pointer instead of the derived const pointer here to mark that this is a command that changes the player object
				players->at(clientIndex)->interruptedPath = false;//only once per interrupt

				NetworkCommunication::addTokenToAllExceptClient(cPlayer->getY(), clientIndex);
				NetworkCommunication::addTokenToAllExceptClient(cPlayer->getX(), clientIndex);
			}
			else if (cPlayer->hasNewPath == true) {
				//Option 2: Transmit new path to clients. SIGNAL -2
				NetworkCommunication::addTokenToAllExceptClient(-2, clientIndex);//signal if new path was found

				players->at(clientIndex)->hasNewPath = false;
				NetworkCommunication::addTokenToAllExceptClient(cPlayer->pathLenght - cPlayer->cPathIndex, clientIndex);//only the path that hasnt been walked yet (lag/connection built up while walking)
				for (int pathIndex = cPlayer->cPathIndex; pathIndex < cPlayer->pathLenght; pathIndex++) {
					NetworkCommunication::addTokenToAllExceptClient(cPlayer->pathXpositions[pathIndex], clientIndex);
					NetworkCommunication::addTokenToAllExceptClient(cPlayer->pathYpositions[pathIndex], clientIndex);
				}
			}
			else {
				//Option 3: Do nothing, either stay on path or stay still. SIGNAL -3
				NetworkCommunication::addTokenToAll(-3);//signal to do nothing
				NetworkCommunication::addTokenToAll(cPlayer->getY());
				NetworkCommunication::addTokenToAll(cPlayer->getX());
			}
			NetworkCommunication::addTokenToAll(cPlayer->getHp());
		}
	}
	else {
		NetworkCommunication::addTokenToAll(myPlayerI);
		const Player* me = players->at(myPlayerI).get();
		if (me->interruptedPath == true) {
			//Option 1: Stop going on path and move to current coordinates of player. SIGNAL -1
			NetworkCommunication::addTokenToAll(-1);//bool if path should be interrupted

			//we use the smart pointer instead of the derived const pointer here to mark that this is a command that changes the player object
			players->at(myPlayerI)->interruptedPath = false;//only once per interrupt

			NetworkCommunication::addTokenToAll(me->getY());
			NetworkCommunication::addTokenToAll(me->getX());
		}
		else if (me->hasNewPath == true) {
			NetworkCommunication::addTokenToAll(-2);//bool if new bath was found

			players->at(myPlayerI)->hasNewPath = false;
			NetworkCommunication::addTokenToAll(me->pathLenght - me->cPathIndex);//only the path that hasnt been walked yet (lag/connection built up while walking)
			for (int pathIndex = me->cPathIndex; pathIndex < me->pathLenght; pathIndex++) {
				NetworkCommunication::addTokenToAll(me->pathXpositions[pathIndex]);
				NetworkCommunication::addTokenToAll(me->pathYpositions[pathIndex]);
			}
		}
		else {
			NetworkCommunication::addTokenToAll(-3);//client doesnt sync coords to host or else it would be double sync which would be stupid
		}
		NetworkCommunication::addTokenToAll(me->getHp());
	}
	GlobalRecources::pfMtx->unlock();//there is no way to do this more efficiently and safe than blocking for the entire transmission
	NetworkCommunication::addTokenToAll(-11);
}

/** Has to pass pathfinding so that we can update pathfinding-graph if player positions changed
**/
int syncCounter = 0;
void PlayerHandling::receivePlayerData(int clientIndex) {
	if (GlobalRecources::isServer == true) {
		int playerIndex = NetworkCommunication::receiveNextToken(clientIndex);

		int actionIndex = NetworkCommunication::receiveNextToken(clientIndex);
		if (actionIndex != -3) {
			std::cout << actionIndex << " ";
		}
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

			if (playerIndex == 1) {
				std::cout << actionIndex;
			}
		}
		else if (actionIndex == -3) {//follow the path given to you
			//do nothing yet
		}
		int hp = NetworkCommunication::receiveNextToken(clientIndex);
		syncCounter ++;
		if (syncCounter % 10 == 0) {
			players->at(playerIndex)->setHp(hp);
			syncCounter = 0;
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
			if (actionIndex != -3) {
				std::cout << playerIndex << ": " << actionIndex << " ";
			}
			if (actionIndex == -4) {//fresh connection, all coords and paths have to be transmitted
				for (int cPlayer = 0; cPlayer < playerCount; cPlayer++) {
					players->at(cPlayer)->setY(NetworkCommunication::receiveNextToken(clientIndex));
					players->at(cPlayer)->setX(NetworkCommunication::receiveNextToken(clientIndex));
				}

				for (int playerI = 0; playerI < playerCount; playerI++) {
					const Player* player = players->at(playerI).get();
					if (NetworkCommunication::receiveNextToken(clientIndex) == -6) {//player has path
						int pathLenght = NetworkCommunication::receiveNextToken(clientIndex);
						vector<int> pathX(pathLenght);
						vector<int> pathY(pathLenght);
						for (int pathIndex = 0; pathIndex < pathLenght; pathIndex++) {
							pathX[pathIndex] = NetworkCommunication::receiveNextToken(clientIndex);
							pathY[pathIndex] = NetworkCommunication::receiveNextToken(clientIndex);
						}
						players->at(playerI)->givePath(move(pathX), move(pathY), pathLenght);
					}
				}
			}
			else if (actionIndex == -1) {//interrupt path/no path is there
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
				int y = NetworkCommunication::receiveNextToken(clientIndex);
				int x = NetworkCommunication::receiveNextToken(clientIndex);
				if (syncCounter % 10 == 0) {
					players->at(playerIndex)->setY(y);
					players->at(playerIndex)->setX(x);
				}
			}
			int hp = NetworkCommunication::receiveNextToken(clientIndex);
			syncCounter++;
			if (syncCounter > 10) {
				players->at(playerIndex)->setHp(hp);
				syncCounter = 0;
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