#include "PathfindingHandler.hpp"
#include <thread>
#include <mutex>
#include "Renderer.hpp"
#include "Terrain.hpp"
#include "Player.hpp"
#include "aStarAlgorithm/Graph.hpp"
#include "aStarAlgorithm/Algorithm.hpp"

	void startPathfindingThread(int goalX, int goalY, int playerIndex, Pathfinding* pathfinding) {
		pathfinding->startPathFinding(goalX, goalY, playerIndex);
	}

	int wRows;
	int wCols;
	Pathfinding::Pathfinding(int worldRows, int worldCols, Terrain* terrain, Player** i_players, int i_playerCount) {
		wRows = worldRows;
		wCols = worldCols;

		players = i_players;
		playerCount = i_playerCount;
		findingPath = false;
		this->goalColToFind = new std::vector<int>();
		this->goalRowToFind = new std::vector<int>();
		this->indicesToFind = new std::vector<int>();

		pathFindingThread = nullptr;
		finishedPathfinding = new std::mutex();

		pathfindingAccuracy = 0.025f;
		//rows and cols are stretched to full screen anyway. Its just accuracy of rendering 
		//and relative coords on which you can orient your drawing. Also makes drawing a rect
		//and stuff easy because width can equal height to make it have sides of the same lenght.


		int pathfindingRows = worldRows * pathfindingAccuracy;//max rows for pathfinding
		int pathfindingCols = worldCols * pathfindingAccuracy;//max cols for pathfinding
		collisionGrid = new bool* [pathfindingRows];
		for (int y = 0; y < pathfindingRows; y++) {
			collisionGrid[y] = new bool[pathfindingCols];
			for (int x = 0; x < pathfindingCols; x++) {
				collisionGrid[y][x] = true;
			}
		}

		//create graph from unmoving solids, can be changed dynamically
		//all player widths and heights are the same so we can just look at index 0
		terrain->addCollidablesToGrid(collisionGrid, pathfindingAccuracy, i_players[0]->getWidth(), i_players[0]->getHeight());
		g = new Graph(worldRows, worldCols, pathfindingAccuracy);
		g->generateWorldGraph(collisionGrid);

		for (int i = 0; i < playerCount; i++) {
			if (players[i]->getHp() > 0) {
				disableArea(players[i]->getRow(), players[i]->getCol(), players[i]->getWidth(), players[i]->getHeight());
			}
		}
	}

	void Pathfinding::pathFindingOnClick(int playerIndex) {
		if (sf::Mouse::isButtonPressed(sf::Mouse::Right) == false) {
			sameClick = false;
		}

		//if rightclick is pressed, find path from player to position of click

		if (sf::Mouse::isButtonPressed(sf::Mouse::Right) == true && sameClick == false && findingPath == false) {
			sameClick = true;
			int mouseX = -1, mouseY = -1;
			Renderer::getMousePos(&mouseX, &mouseY, true);//writes mouse coords into mouseX, mouseY
			if (mouseX != -1) {//stays at -1 if click is outside of window
				findPath(mouseX, mouseY, playerIndex);
			}
		}
	}

	void Pathfinding::findPath(int goalX, int goalY, int playerIndex) {
		finishedPathfinding->try_lock();

		if (findingPath == false) {
			findingPath = true;
			players[playerIndex]->setFindingPath(true);
			if (players[playerIndex]->hasPath() == true) {
				players[playerIndex]->deletePath();
			}
			finishedPathfinding->unlock();
			pathFindingThread = new std::thread(&startPathfindingThread, goalX, goalY, playerIndex, this);
		}
		else {
			goalColToFind->push_back(goalX);
			goalRowToFind->push_back(goalY);
			indicesToFind->push_back(playerIndex);
			finishedPathfinding->unlock();
		}
	}

	void Pathfinding::workThroughPathfindingQueue() {
		if (goalColToFind->size() > 0) {
			finishedPathfinding->lock();
			if (findingPath == false) {
				finishedPathfinding->unlock();
				findPath(goalColToFind->at(0), goalRowToFind->at(0), indicesToFind->at(0));
				goalColToFind->erase(goalColToFind->begin());
				goalRowToFind->erase(goalRowToFind->begin());
				indicesToFind->erase(indicesToFind->begin());
			}
			else {
				finishedPathfinding->unlock();
			}
		}
	}



	void Pathfinding::moveObjects() {
		g->debugDrawing();
		workThroughPathfindingQueue();
		//this->enableArea(0, 0, wCols - 1, wRows - 1);//enable all
		for (int i = 0; i < playerCount; i++) {
			if (players[i]->getHp() > 0) {
				finishedPathfinding->lock();
				if (players[i]->hasPath() == true) {
					int tempRow = players[i]->getRow();
					int tempCol = players[i]->getCol();

					players[i]->move();//if he has a path, he moves on this path every 1 / velocity iterations of eventloop

					this->enableArea(tempRow, tempCol, players[0]->getWidth() + 100, players[0]->getHeight() + 100);//enable old position
					this->disableArea(players[i]->getRow(), players[i]->getCol(), players[0]->getWidth(), players[0]->getHeight());//disable new position


					finishedPathfinding->unlock();
					//for efficiency only find new path if you move next to a possibly moving object
					//IF PLAYER POSITION CHANGED THIS FRAME and players are close to each other find new paths for other players
					if (players[i]->getRow() != tempRow && players[i]->getCol() != tempCol) {//only having a path doesnt mean you moved on it
						//find new paths for players close to this player
						this->playerInteraction(i);
					}

				}
				else {
					this->disableArea(players[i]->getRow(), players[i]->getCol(), players[i]->getWidth(), players[i]->getHeight());
					finishedPathfinding->unlock();
				}
			}
		}
	}

	void Pathfinding::playerInteraction(int movedPlayerIndex) {
		//find new paths for players close to this player
		Player* movedPlayer = players[movedPlayerIndex];
		bool unlock = true;
		for (int j = 0; j < playerCount; j++) {
			if (players[j]->getHp() > 0) {
				if (j != movedPlayerIndex) {
					finishedPathfinding->lock();
					Player* cPlayer = players[j];
					if (cPlayer->hasPath() == true) {
						//did player position change?

						for (int i = 0; i < cPlayer->pathLenght; i++) {
							int col = cPlayer->pathXpositions[i];
							int row = cPlayer->pathYpositions[i];
							if (row > movedPlayer->getRow() - players[0]->getHeight() && row < movedPlayer->getRow() + players[0]->getHeight()) {
								if (col > movedPlayer->getCol() - players[0]->getWidth() && col < movedPlayer->getCol() + players[0]->getWidth()) {
									int col = cPlayer->getPathGoalX();
									int row = cPlayer->getPathGoalY();
									this->findPath(col, row, j);
									unlock = false;
									break;
								}
							}
						}

						//if (abs((cPlayer->getRow() + (players[0]->getHeight() / 2)) - (movedPlayer->getRow() + (players[0]->getHeight() / 2))) < players[0]->getHeight() * 3
						//	&& abs((cPlayer->getCol() + (players[0]->getWidth() / 2)) - (movedPlayer->getCol() + (players[0]->getWidth() / 2))) < players[0]->getWidth() * 3) {
							//find new path to same goal

						//}
					}
					if (unlock == true) {
						finishedPathfinding->unlock();
					}
				}
			}
		}
	}


	void Pathfinding::disableArea(int row, int col, int width, int height) {
		if (findingPath == false) {
			g->disableObjectBounds(row, col, width, height);
		}
	}

	void Pathfinding::enableArea(int row, int col, int width, int height) {
		if (findingPath == false) {
			g->enableObjectBounds(row, col, width, height);
		}
	}











	void Pathfinding::startPathFinding(int goalX, int goalY, int playerIndex) {

		int* xPath = nullptr;
		int* yPath = nullptr;
		int pathlenght = 0;

		int mouseRow = goalY;
		int mouseCol = goalX;

		Player* player = players[playerIndex];


		//Cant do that in the other thread while pathfinding so we needa do it here
		finishedPathfinding->lock();


		g->enableObjectBounds(player->getRow(), player->getCol(), player->getWidth(), player->getHeight());
		
		for (int i = 0; i < playerCount; i++) {
			if (players[i]->getHp() > 0) {
				if (i != playerIndex) {
					Player* cPlayer = players[i];
					g->disableObjectBounds(cPlayer->getRow(), cPlayer->getCol(), cPlayer->getWidth(), cPlayer->getHeight());
				}
			}
		}

		finishedPathfinding->unlock();
		bool found = Algorithm::findPath(&xPath, &yPath, &pathlenght, g, player->getRow(), player->getCol(), mouseRow, mouseCol);

		//reverse accuracy simplification
		for (int i = 0; i < pathlenght; i++) {
			xPath[i] /= pathfindingAccuracy;
			yPath[i] /= pathfindingAccuracy;
		}

		finishedPathfinding->lock();
		
		if (found == true) {
			player->givePath(xPath, yPath, pathlenght);
		}
		findingPath = false;
		player->setFindingPath(false);
		finishedPathfinding->unlock();

		pathFindingThread->detach();
		delete pathFindingThread;
	}