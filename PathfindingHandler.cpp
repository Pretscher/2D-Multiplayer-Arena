#include "PathfindingHandler.hpp"
#include <thread>
#include <mutex>
#include "Renderer.hpp"
#include "Terrain.hpp"
#include "Player.hpp"
#include "Graph.hpp"
#include "Algorithm.hpp"
#include "GlobalRecources.hpp"
#include "iostream" 
using namespace std;

void startPathfindingThread(Pathfinding* pathfinding) {
	pathfinding->startPathFinding();
	
}

unique_ptr<Algorithm> aStar;
Pathfinding::Pathfinding() {
	cgoalX = -1;
	cgoalY = -1;
	cPlayerIndex = -1;
	newPathFinding = false;
	
	wYs = GlobalRecources::worldHeight;
	wXs = GlobalRecources::worldWidth;

	players = GlobalRecources::players;
	findingPath = false;
	this->goalXToFind = vector<int>();
	this->goalYToFind = vector<int>();
	this->indicesToFind = vector<int>();
	this->newGoalXs = unique_ptr<vector<int>>(new vector<int>());
	this->newGoalYs = unique_ptr<vector<int>>(new vector<int>());

	pathFindingThread = new thread(&startPathfindingThread, this);
	pfMtx = shared_ptr<mutex>(new mutex());

	pathfindingAccuracy = 0.025f;
	//ys and xs are stretched to full screen anyway. Its just accuracy of rendering 
	//and relative coords on which you can orient your drawing. Also makes drawing a rect
	//and stuff easy because width can equal height to make it have sides of the same lenght.


	int pathfindingYs = wYs * pathfindingAccuracy;//max ys for pathfinding
	int pathfindingXs = wXs * pathfindingAccuracy;//max xs for pathfinding
	colisionGrid = new bool*[pathfindingYs];
	for (int y = 0; y < pathfindingYs; y++) {
		colisionGrid[y] = new bool[pathfindingXs];
		for (int x = 0; x < pathfindingXs; x++) {
			colisionGrid[y][x] = true;
		}
	}

	//create graph from unmoving solids, can be changed dynamically
	//all player widths and heights are the same so we can just look at index 0
	GlobalRecources::terrain->addCollidablesToGrid(colisionGrid, pathfindingAccuracy, players->at(0)->getWidth(), players->at(0)->getHeight());
	g = shared_ptr<Graph>(new Graph(wYs, wXs, pathfindingAccuracy));
	g->generateWorldGraph(colisionGrid);

	GlobalRecources::pfMtx = pfMtx;
	GlobalRecources::pFinding = this;

	aStar = unique_ptr<Algorithm>(new Algorithm(g, pfMtx));
}

void Pathfinding::pathFindingOnClick() {
	if (sf::Mouse::isButtonPressed(sf::Mouse::Right) == false) {
		sameClick = false;
	}

	//if rightclick is pressed, find path from player to position of click

	if (sf::Mouse::isButtonPressed(sf::Mouse::Right) == true && sameClick == false && getFindingPath() == false) {
		sameClick = true;
		int mouseY = -1, mouseX = -1;
		
		Renderer:: getMousePos(mouseX, mouseY, true, true);//writes mouse coords into mouseX, mouseY
		if (mouseY != -1) {//stays at -1 if click is outside of window
			if (mouseX - players->at(0)->getWidth() / 2 > 0) {
				mouseX -= players->at(0)->getWidth() / 2;
			}
			if (mouseY - players->at(0)->getHeight() / 2 > 0) {
				mouseY -= players->at(0)->getHeight() / 2;
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
	for (int i = 0; i < players->size(); i++) {
		if (players->at(i)->targetAble == false) {
			if (i == myPlayerIndex) {
				for (int j = 0; j < players->size(); j++) {
					enablePlayer(j, false);//untargetable players walk through everyone
				}
			}
			else {
				enablePlayer(i, false);//you can walk on untargetable players
			}
		}
	}
	int pY = players->at(playerIndex)->getY();
	int pX = players->at(playerIndex)->getX();
	g->findNextUseableCoords(&pX, &pY, true);
	players->at(playerIndex)->setY(pY);
	players->at(playerIndex)->setX(pX);

	g->findNextUseableCoords(&goalX, &goalY, true);
	disablePlayer(myPlayerIndex);

	if (goalX == pX && goalY == pY) {//player has valid coords and is at goal, so we can return after disabling palyers again
		return;
	}

	if (getFindingPath() == false) {
		setFindingPath(true);
		players->at(playerIndex)->setFindingPath(true);
		if (players->at(playerIndex)->hasPath() == true) {
			players->at(playerIndex)->deletePath();
		}
		setNewPathfinding(true);
		cgoalX = goalX;
		cgoalY = goalY;

		cPlayerIndex = playerIndex;
	}
	else {
		goalXToFind.push_back(goalX);
		goalYToFind.push_back(goalY);
		indicesToFind.push_back(playerIndex);
	}
}

void Pathfinding::workThroughPathfindingQueue() {
	if (goalXToFind.size() > 0) {
		if (getFindingPath() == false) {
			findPath(goalXToFind.at(0), goalYToFind.at(0), indicesToFind.at(0));
			goalXToFind.erase(goalXToFind.begin());
			goalYToFind.erase(goalYToFind.begin());
			indicesToFind.erase(indicesToFind.begin());
		}
	}
}



void Pathfinding::moveObjects() {
	workThroughPathfindingQueue();
	//this->enableArea(0, 0, wXs - 1, wYs - 1);//enable all
	for (int i = 0; i < players->size(); i++) {
		if (players->at(i)->getHp() > 0) {
			if (players->at(i)->hasPath() == true) {
				int tempY = players->at(i)->getY();
				int tempX = players->at(i)->getX();
				enablePlayer(i, true);
				players->at(i)->move();//if he has a path, he moves on this path every 1 / velocity iterations of eventloop
				disablePlayer(i);//disable new position
				//for efficiency only find new path if you move next to a possibly moving object
				//IF PLAYER POSITION CHANGED THIS FRAME and players are close to each other find new paths for other players
				if (players->at(i)->getY() != tempY && players->at(i)->getX() != tempX) {//only having a path doesnt mean you moved on it
					//find new paths for players close to this player
					this->playerInteraction(i);
				}
			}
			else {
				disablePlayer(i);
			}
		}
	}
}

void Pathfinding::playerInteraction(int movedPlayerIndex) {
	//find new paths for players close to this player
	const Player* movedPlayer = players->at(movedPlayerIndex).get();
	for (int j = 0; j < players->size(); j++) {
		if (players->at(j)->getHp() > 0) {
			if (j != movedPlayerIndex) {
				const Player* cPlayer = players->at(j).get();
				if (cPlayer->hasPath() == true) {
					//if paths cross each other, disable all crossing points and find new path. this is periodically called

					vector<int>* disabledYs = nullptr;
					vector<int>* disabledXs = nullptr;
					for (int i = 0; i < cPlayer->pathLenght; i++) {
						int x = cPlayer->pathXpositions[i];
						int y = cPlayer->pathYpositions[i];
						if (y > movedPlayer->getY() - players->at(0)->getHeight() && y < movedPlayer->getY() + players->at(0)->getHeight()) {
							if (x > movedPlayer->getX() - players->at(0)->getWidth() && x < movedPlayer->getX() + players->at(0)->getWidth()) {
								int x = cPlayer->getPathgoalX();
								int y = cPlayer->getPathgoalY();
								
								g->disableObjectBounds(y - players->at(0)->getHeight(), x - players->at(0)->getWidth(), 
										players->at(0)->getWidth(), players->at(0)->getHeight());//disable new position

								if (disabledYs == nullptr) {
									disabledYs = new vector<int>();
									disabledXs = new vector<int>();
								}
								disabledYs->push_back(y);
								disabledXs->push_back(x);
								break;
							}
						}
					}
					
					if (disabledYs != nullptr) {
						this->findPath(players->at(j)->getPathgoalX(), players->at(j)->getPathgoalY(), j);
						for (int i = 0; i < disabledYs->size(); i++) {
							g->enableObjectBounds(disabledYs->at(i) - players->at(0)->getHeight(), disabledXs->at(i) - players->at(0)->getWidth(),
									players->at(0)->getWidth(), players->at(0)->getHeight());//enable old position
						}
						delete disabledYs;
						delete disabledXs;
					}
				}
			}
		}
	}
}


//enables player coords and after that disables all coords of other movables again in case their area was affected by that.
void Pathfinding::enablePlayer(int i_playerIndex, bool disableOthers) {
	const Player* player = players->at(i_playerIndex).get();

	g->enableObjectBounds(player->getY() - 150, player->getX() - 150, player->getWidth() + 200, player->getHeight() + 200);

	if (disableOthers == true) {
		for (int i = 0; i < players->size(); i++) {
			if (players->at(i)->getHp() > 0) {
				if (i != i_playerIndex) {
					disablePlayer(i);
				}
			}
		}
	}
}

//enables player coords and after that disables all coords of other movables 			setNewPathfinding(false);again in case their area was affected by that.
void Pathfinding::disablePlayer(int i_playerIndex) {
	const Player* player = players->at(i_playerIndex).get();
	g->disableObjectBounds(player->getY() - 100, player->getX() - 100, player->getWidth() + 100, player->getHeight() + 100);
}






void Pathfinding::startPathFinding() {
	while (true) {
		this_thread::sleep_for(chrono::milliseconds(5));

		if (this->getNewPathfinding() == true) {
			vector<int> pathXs;
			vector<int> pathYs;
			int pathlenght = 0;
			const Player* player = players->at(cPlayerIndex).get();

			enablePlayer(cPlayerIndex, true);
			for (int i = 0; i < players->size(); i++) {
				if (players->at(i)->targetAble == false) {
					if (i == cPlayerIndex) {
						for (int j = 0; j < players->size(); j++) {
							enablePlayer(j, false);//untargetable players walk through everyone
						}
					}
					else {
						enablePlayer(i, false);//you can walk on untargetable players
					}
				}
			}

			bool found = aStar->findPath(pathYs, pathXs, pathlenght, player->getY() + (player->getHeight() / 2),
															 player->getX() + (player->getWidth() / 2), cgoalY, cgoalX);

			disablePlayer(cPlayerIndex);

			//reverse accuracy simplification
			for (int i = 0; i < pathlenght - 1; i++) {
				pathXs[i] = (float) pathXs[i] / pathfindingAccuracy;
				pathYs[i] = (float) pathYs[i] / pathfindingAccuracy;
			}

			if (found == true) {
				players->at(cPlayerIndex)->givePath(move(pathXs), move(pathYs), pathlenght);//move, we dont need this anymore in this func
			}
			players->at(cPlayerIndex)->setFindingPath(false);
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
