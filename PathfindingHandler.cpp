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

Pathfinding::Pathfinding(int worldRows, int worldCols, Terrain* terrain, Player** i_players, int i_playerCount) {
	cgoalCol = -1;
	cgoalRow = -1;
	cPlayerIndex = -1;
	newPathFinding = false;
	
	wRows = worldRows;
	wCols = worldCols;

	players = i_players;
	playerCount = i_playerCount;
	findingPath = false;
	this->goalColToFind = new std::vector<int>();
	this->goalRowToFind = new std::vector<int>();
	this->indicesToFind = new std::vector<int>();

	pathFindingThread = new std::thread(&startPathfindingThread, this);
	pfMtx = new std::mutex();

	pathfindingAccuracy = 0.025f;
	//rows and cols are stretched to full screen anyway. Its just accuracy of rendering 
	//and relative coords on which you can orient your drawing. Also makes drawing a rect
	//and stuff easy because width can equal height to make it have sides of the same lenght.


	int pathfindingRows = worldRows * pathfindingAccuracy;//max rows for pathfinding
	int pathfindingCols = worldCols * pathfindingAccuracy;//max cols for pathfinding
	collisionGrid = new bool*[pathfindingRows];
	for (int row = 0; row < pathfindingRows; row++) {
		collisionGrid[row] = new bool[pathfindingCols];
		for (int col = 0; col < pathfindingCols; col++) {
			collisionGrid[row][col] = true;
		}
	}

	//create graph from unmoving solids, can be changed dynamically
	//all player widths and heights are the same so we can just look at index 0
	terrain->addCollidablesToGrid(collisionGrid, pathfindingAccuracy, i_players[0]->getWidth(), i_players[0]->getHeight());
	g = new Graph(worldRows, worldCols, pathfindingAccuracy);
	g->generateWorldGraph(collisionGrid);
}

void Pathfinding::pathFindingOnClick() {
	if (sf::Mouse::isButtonPressed(sf::Mouse::Right) == false) {
		sameClick = false;
	}

	//if rightclick is pressed, find path from player to position of click

	if (sf::Mouse::isButtonPressed(sf::Mouse::Right) == true && sameClick == false && getFindingPath() == false) {
		sameClick = true;
		int mouseRow = -1, mouseCol = -1;
		Renderer::getMousePos(&mouseRow, &mouseCol, true, true);//writes mouse coords into mouseX, mouseY
		if (mouseRow != -1) {//stays at -1 if click is outside of window
			if (mouseCol - players[0]->getWidth() / 2 > 0) {
				mouseCol -= players[0]->getWidth() / 2;
			}
			if (mouseRow - players[0]->getHeight() / 2 > 0) {
				mouseRow -= players[0]->getHeight() / 2;
			}
			findPath(mouseCol, mouseRow, myPlayerIndex);
		}
	}
}

void Pathfinding::update() {
	pathFindingOnClick();
	moveObjects();
}

void Pathfinding::findPath(int goalCol, int goalRow, int playerIndex) {

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
	int pRow = players[playerIndex]->getRow();
	int pCol = players[playerIndex]->getCol();
	g->findNextUseableCoords(&pCol, &pRow, true);
	players[playerIndex]->setRow(pRow);
	players[playerIndex]->setCol(pCol);

	g->findNextUseableCoords(&goalCol, &goalRow, true);
	disablePlayer(myPlayerIndex);

	if (goalCol == pCol && goalRow == pRow) {//player has valid coords and is at goal, so we can return after disabling palyers again
		return;
	}

	if (getFindingPath() == false) {
		setFindingPath(true);
		players[playerIndex]->setFindingPath(true);
		if (players[playerIndex]->hasPath() == true) {
			players[playerIndex]->deletePath();
		}
		setNewPathfinding(true);
		cgoalCol = goalCol;
		cgoalRow = goalRow;

		cPlayerIndex = playerIndex;
	}
	else {
		goalColToFind->push_back(goalCol);
		goalRowToFind->push_back(goalRow);
		indicesToFind->push_back(playerIndex);
	}
}

void Pathfinding::workThroughPathfindingQueue() {
	if (goalColToFind->size() > 0) {
		if (getFindingPath() == false) {
			findPath(goalColToFind->at(0), goalRowToFind->at(0), indicesToFind->at(0));
			goalColToFind->erase(goalColToFind->begin());
			goalRowToFind->erase(goalRowToFind->begin());
			indicesToFind->erase(indicesToFind->begin());
		}
	}
}



void Pathfinding::moveObjects() {
	workThroughPathfindingQueue();
	//this->enableArea(0, 0, wCols - 1, wRows - 1);//enable all
	for (int i = 0; i < playerCount; i++) {
		if (players[i]->getHp() > 0) {
			if (players[i]->hasPath() == true) {
				int tempRow = players[i]->getRow();
				int tempCol = players[i]->getCol();
				players[i]->move();//if he has a path, he moves on this path every 1 / velocity iterations of eventloop
				this->enableArea(tempRow, tempCol, players[0]->getWidth() + 100, players[0]->getHeight() + 100);//enable old position
				this->disableArea(players[i]->getRow(), players[i]->getCol(), players[0]->getWidth(), players[0]->getHeight());//disable new position
				//for efficiency only find new path if you move next to a possibly moving object
				//IF PLAYER POSITION CHANGED THIS FRAME and players are close to each other find new paths for other players
				if (players[i]->getRow() != tempRow && players[i]->getCol() != tempCol) {//only having a path doesnt mean you moved on it
					//find new paths for players close to this player
					this->playerInteraction(i);
				}
			}
			else {
				this->disableArea(players[i]->getRow(), players[i]->getCol(), players[i]->getWidth(), players[i]->getHeight());
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

					std::vector<int>* disabledRows = nullptr;
					std::vector<int>* disabledCols = nullptr;
					for (int i = 0; i < cPlayer->pathLenght; i++) {
						int col = cPlayer->pathXpositions[i];
						int row = cPlayer->pathYpositions[i];
						if (row > movedPlayer->getRow() - players[0]->getHeight() && row < movedPlayer->getRow() + players[0]->getHeight()) {
							if (col > movedPlayer->getCol() - players[0]->getWidth() && col < movedPlayer->getCol() + players[0]->getWidth()) {
								int col = cPlayer->getPathgoalCol();
								int row = cPlayer->getPathgoalRow();
								
								this->disableArea(row - players[0]->getHeight(), col - players[0]->getWidth(), players[0]->getWidth(), players[0]->getHeight());//disable new position

								if (disabledRows == nullptr) {
									disabledRows = new std::vector<int>();
									disabledCols = new std::vector<int>();
								}
								disabledRows->push_back(row);
								disabledCols->push_back(col);
								break;
							}
						}
					}
					
					if (disabledRows != nullptr) {
						this->findPath(players[j]->getPathgoalCol(), players[j]->getPathgoalRow(), j);
						for (int i = 0; i < disabledRows->size(); i++) {
							this->enableArea(disabledRows->at(i) - players[0]->getHeight(), disabledCols->at(i) - players[0]->getWidth(), players[0]->getWidth(), players[0]->getHeight());//enable old position
						}
						delete disabledRows;
						delete disabledCols;
					}
				}
			}
		}
	}
}


void Pathfinding::disableArea(int row, int col, int width, int height) {
	if (getFindingPath() == false) {
		g->disableObjectBounds(row, col, width, height);
	}
}

void Pathfinding::enableArea(int row, int col, int width, int height) {
	if (getFindingPath() == false) {
		g->enableObjectBounds(row, col, width, height);
	}
}

//enables player coords and after that disables all coords of other movables again in case their area was affected by that.
void Pathfinding::enablePlayer(int i_playerIndex, bool disableOthers) {
	Player* player = players[i_playerIndex];

	g->enableObjectBounds(player->getRow(), player->getCol(), player->getWidth(), player->getHeight());

	if (disableOthers == true) {
		for (int i = 0; i < playerCount; i++) {
			if (players[i]->getHp() > 0) {
				if (i != cPlayerIndex) {
					Player* cPlayer = players[i];
					g->disableObjectBounds(cPlayer->getRow(), cPlayer->getCol(), cPlayer->getWidth(), cPlayer->getHeight());
				}
			}
		}
	}
}

//enables player coords and after that disables all coords of other movables 			setNewPathfinding(false);again in case their area was affected by that.
void Pathfinding::disablePlayer(int i_playerIndex) {
	Player* player = players[i_playerIndex];
	g->disableObjectBounds(player->getRow(), player->getCol(), player->getWidth(), player->getHeight());
}







void Pathfinding::startPathFinding() {
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(5));

		if (this->getNewPathfinding() == true) {
			int* pathCols = nullptr;
			int* pathRows = nullptr;
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

			bool found = Algorithm::findPath(&pathRows, &pathCols, &pathlenght, g, player->getRow() + (player->getHeight() / 2),
															 player->getCol() + (player->getWidth() / 2), cgoalRow, cgoalCol);

			disablePlayer(cPlayerIndex);

			//reverse accuracy simplification
			for (int i = 0; i < pathlenght - 1; i++) {
				pathCols[i] = (float) pathCols[i] / pathfindingAccuracy;
				pathRows[i] = (float) pathRows[i] / pathfindingAccuracy;
			}

			if (found == true) {
				player->givePath(pathCols, pathRows, pathlenght);
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
