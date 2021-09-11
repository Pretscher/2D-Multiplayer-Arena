#include "PathfindingHandler.hpp"
#include <thread>
#include <mutex>
#include "Renderer.hpp"
#include "Terrain.hpp"
#include "Player.hpp"
#include "Graph.hpp"
#include "Algorithm.hpp"

void startPathfindingThread(Pathfinding* pathfinding) {
	pathfinding->startPathFinding();
}

Pathfinding::Pathfinding(int worldYs, int worldXs, Terrain* terrain, Player** i_players, int i_playerCount) {
	cgoalX = -1;
	cgoalY = -1;
	cPlayerIndex = -1;
	newPathFinding = false;
	
	wYs = worldYs;
	wXs = worldXs;

	players = i_players;
	playerCount = i_playerCount;
	findingPath = false;
	this->goalXToFind = new std::vector<int>();
	this->goalYToFind = new std::vector<int>();
	this->indicesToFind = new std::vector<int>();

	pathFindingThread = new std::thread(&startPathfindingThread, this);
	pfMtx = new std::mutex();

	pathfindingAccuracy = 0.025f;
	//ys and xs are stretched to full screen anyway. Its just accuracy of rendering 
	//and relative coords on which you can orient your drawing. Also makes drawing a rect
	//and stuff easy because width can equal height to make it have sides of the same lenght.


	int pathfindingYs = worldYs * pathfindingAccuracy;//max ys for pathfinding
	int pathfindingXs = worldXs * pathfindingAccuracy;//max xs for pathfinding
	xlisionGrid = new bool*[pathfindingYs];
	for (int y = 0; y < pathfindingYs; y++) {
		xlisionGrid[y] = new bool[pathfindingXs];
		for (int x = 0; x < pathfindingXs; x++) {
			xlisionGrid[y][x] = true;
		}
	}

	//create graph from unmoving solids, can be changed dynamically
	//all player widths and heights are the same so we can just look at index 0
	terrain->addXlidablesToGrid(xlisionGrid, pathfindingAccuracy, i_players[0]->getWidth(), i_players[0]->getHeight());
	g = new Graph(worldYs, worldXs, pathfindingAccuracy);
	g->generateWorldGraph(xlisionGrid);
}

void Pathfinding::pathFindingOnClick() {
	if (sf::Mouse::isButtonPressed(sf::Mouse::Right) == false) {
		sameClick = false;
	}

	//if rightclick is pressed, find path from player to position of click

	if (sf::Mouse::isButtonPressed(sf::Mouse::Right) == true && sameClick == false && getFindingPath() == false) {
		sameClick = true;
		int mouseY = -1, mouseX = -1;
		Renderer::getMousePos(&mouseY, &mouseX, true, true);//writes mouse coords into mouseX, mouseY
		if (mouseY != -1) {//stays at -1 if click is outside of window
			if (mouseX - players[0]->getWidth() / 2 > 0) {
				mouseX -= players[0]->getWidth() / 2;
			}
			if (mouseY - players[0]->getHeight() / 2 > 0) {
				mouseY -= players[0]->getHeight() / 2;
			}
			findPath(mouseX, mouseY, myPlayerIndex);
		}
	}
}

void Pathfinding::update() {
	pathFindingOnClick();
	moveObjects();
}

void Pathfinding::findPath(int goalX, int goalY, int playerIndex) {

	enablePlayer(myPlayerIndex, true);
	for (int i = 0; i < playerCount; i++) {
		if (players[i]->targetAble == false) {
			if (i == myPlayerIndex) {
				for (int j = 0; j < playerCount; j++) {
					enablePlayer(j, false);//untargetable players walk through everyone
				}
			}
			else {
				enablePlayer(i, false);//you can walk on untargetable players
			}
		}
	}
	int pY = players[playerIndex]->getY();
	int pX = players[playerIndex]->getX();
	g->findNextUseableCoords(&pX, &pY, true);
	players[playerIndex]->setY(pY);
	players[playerIndex]->setX(pX);

	g->findNextUseableCoords(&goalX, &goalY, true);
	disablePlayer(myPlayerIndex);

	if (goalX == pX && goalY == pY) {//player has valid coords and is at goal, so we can return after disabling palyers again
		return;
	}

	if (getFindingPath() == false) {
		setFindingPath(true);
		players[playerIndex]->setFindingPath(true);
		if (players[playerIndex]->hasPath() == true) {
			players[playerIndex]->deletePath();
		}
		setNewPathfinding(true);
		cgoalX = goalX;
		cgoalY = goalY;

		cPlayerIndex = playerIndex;
	}
	else {
		goalXToFind->push_back(goalX);
		goalYToFind->push_back(goalY);
		indicesToFind->push_back(playerIndex);
	}
}

void Pathfinding::workThroughPathfindingQueue() {
	if (goalXToFind->size() > 0) {
		if (getFindingPath() == false) {
			findPath(goalXToFind->at(0), goalYToFind->at(0), indicesToFind->at(0));
			goalXToFind->erase(goalXToFind->begin());
			goalYToFind->erase(goalYToFind->begin());
			indicesToFind->erase(indicesToFind->begin());
		}
	}
}



void Pathfinding::moveObjects() {
	workThroughPathfindingQueue();
	//this->enableArea(0, 0, wXs - 1, wYs - 1);//enable all
	for (int i = 0; i < playerCount; i++) {
		if (players[i]->getHp() > 0) {
			if (players[i]->hasPath() == true) {
				int tempY = players[i]->getY();
				int tempX = players[i]->getX();
				players[i]->move();//if he has a path, he moves on this path every 1 / velocity iterations of eventloop
				this->enableArea(tempY, tempX, players[0]->getWidth() + 100, players[0]->getHeight() + 100);//enable old position
				this->disableArea(players[i]->getY(), players[i]->getX(), players[0]->getWidth(), players[0]->getHeight());//disable new position
				//for efficiency only find new path if you move next to a possibly moving object
				//IF PLAYER POSITION CHANGED THIS FRAME and players are close to each other find new paths for other players
				if (players[i]->getY() != tempY && players[i]->getX() != tempX) {//only having a path doesnt mean you moved on it
					//find new paths for players close to this player
					this->playerInteraction(i);
				}
			}
			else {
				this->disableArea(players[i]->getY(), players[i]->getX(), players[i]->getWidth(), players[i]->getHeight());
			}
		}
	}
}

void Pathfinding::playerInteraction(int movedPlayerIndex) {
	//find new paths for players close to this player
	Player* movedPlayer = players[movedPlayerIndex];
	for (int j = 0; j < playerCount; j++) {
		if (players[j]->getHp() > 0) {
			if (j != movedPlayerIndex) {
				Player* cPlayer = players[j];
				if (cPlayer->hasPath() == true) {
					//if paths cross each other, disable all crossing points and find new path. this is periodically called

					std::vector<int>* disabledYs = nullptr;
					std::vector<int>* disabledXs = nullptr;
					for (int i = 0; i < cPlayer->pathLenght; i++) {
						int x = cPlayer->pathXpositions[i];
						int y = cPlayer->pathYpositions[i];
						if (y > movedPlayer->getY() - players[0]->getHeight() && y < movedPlayer->getY() + players[0]->getHeight()) {
							if (x > movedPlayer->getX() - players[0]->getWidth() && x < movedPlayer->getX() + players[0]->getWidth()) {
								int x = cPlayer->getPathgoalX();
								int y = cPlayer->getPathgoalY();
								
								this->disableArea(y - players[0]->getHeight(), x - players[0]->getWidth(), players[0]->getWidth(), players[0]->getHeight());//disable new position

								if (disabledYs == nullptr) {
									disabledYs = new std::vector<int>();
									disabledXs = new std::vector<int>();
								}
								disabledYs->push_back(y);
								disabledXs->push_back(x);
								break;
							}
						}
					}
					
					if (disabledYs != nullptr) {
						this->findPath(players[j]->getPathgoalX(), players[j]->getPathgoalY(), j);
						for (int i = 0; i < disabledYs->size(); i++) {
							this->enableArea(disabledYs->at(i) - players[0]->getHeight(), disabledXs->at(i) - players[0]->getWidth(), players[0]->getWidth(), players[0]->getHeight());//enable old position
						}
						delete disabledYs;
						delete disabledXs;
					}
				}
			}
		}
	}
}


void Pathfinding::disableArea(int y, int x, int width, int height) {
	if (getFindingPath() == false) {
		g->disableObjectBounds(y, x, width, height);
	}
}

void Pathfinding::enableArea(int y, int x, int width, int height) {
	if (getFindingPath() == false) {
		g->enableObjectBounds(y, x, width, height);
	}
}

//enables player coords and after that disables all coords of other movables again in case their area was affected by that.
void Pathfinding::enablePlayer(int i_playerIndex, bool disableOthers) {
	Player* player = players[i_playerIndex];

	g->enableObjectBounds(player->getY(), player->getX(), player->getWidth(), player->getHeight());

	if (disableOthers == true) {
		for (int i = 0; i < playerCount; i++) {
			if (players[i]->getHp() > 0) {
				if (i != cPlayerIndex) {
					Player* cPlayer = players[i];
					g->disableObjectBounds(cPlayer->getY(), cPlayer->getX(), cPlayer->getWidth(), cPlayer->getHeight());
				}
			}
		}
	}
}

//enables player coords and after that disables all coords of other movables 			setNewPathfinding(false);again in case their area was affected by that.
void Pathfinding::disablePlayer(int i_playerIndex) {
	Player* player = players[i_playerIndex];
	g->disableObjectBounds(player->getY(), player->getX(), player->getWidth(), player->getHeight());
}







void Pathfinding::startPathFinding() {
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(5));

		if (this->getNewPathfinding() == true) {
			int* pathXs = nullptr;
			int* pathYs = nullptr;
			int pathlenght = 0;
			Player* player = players[cPlayerIndex];

			enablePlayer(cPlayerIndex, true);
			for (int i = 0; i < playerCount; i++) {
				if (players[i]->targetAble == false) {
					if (i == cPlayerIndex) {
						for (int j = 0; j < playerCount; j++) {
							enablePlayer(j, false);//untargetable players walk through everyone
						}
					}
					else {
						enablePlayer(i, false);//you can walk on untargetable players
					}
				}
			}

			bool found = Algorithm::findPath(&pathYs, &pathXs, &pathlenght, g, player->getY() + (player->getHeight() / 2),
															 player->getX() + (player->getWidth() / 2), cgoalY, cgoalX);

			disablePlayer(cPlayerIndex);

			//reverse accuracy simplification
			for (int i = 0; i < pathlenght - 1; i++) {
				pathXs[i] = (float) pathXs[i] / pathfindingAccuracy;
				pathYs[i] = (float) pathYs[i] / pathfindingAccuracy;
			}

			if (found == true) {
				player->givePath(pathXs, pathYs, pathlenght);
			}
			player->setFindingPath(false);
			setNewPathfinding(false);
			setFindingPath(false);
		}
	}
}

//thread safety: this var should only be written through this setter, but you dont have to handle the mutex yourself
void Pathfinding::setNewPathfinding(bool i_newPf) {
	pfMtx->lock();
	newPathFinding = i_newPf;
	pfMtx->unlock();
}

//thread safety: this var should only be accessed through this getter, but you dont have to handle the mutex yourself
bool Pathfinding::getNewPathfinding() {
	bool temp;
	pfMtx->lock();
	temp = newPathFinding;
	pfMtx->unlock();
	return temp;
}

void Pathfinding::setFindingPath(bool i_newPf) {
	pfMtx->lock();
	findingPath = i_newPf;
	pfMtx->unlock();
}

//thread safety: this var should only be accessed through this getter, but you dont have to handle the mutex yourself
bool Pathfinding::getFindingPath() {
	bool temp;
	pfMtx->lock();
	temp = findingPath;
	pfMtx->unlock();
	return temp;
}
