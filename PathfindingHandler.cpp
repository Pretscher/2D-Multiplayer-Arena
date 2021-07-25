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

	Pathfinding::Pathfinding(int worldRows, int worldCols, Terrain* terrain, Player** i_players, int i_playerCount) {
		players = i_players;
		playerCount = i_playerCount;

		pathFindingThread = nullptr;
		finishedPathfinding = new std::mutex();

		pathfindingAccuracy = 0.05f;
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
		g = new Graph(worldRows, worldCols, 0.05f);
		g->generateWorldGraph(collisionGrid);

		for (int i = 0; i < playerCount; i++) {
			disableArea(players[i]->getRow(), players[i]->getCol(), players[i]->getWidth(), players[i]->getHeight());
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
		finishedPathfinding->lock();
		if (findingPath == false) {
			findingPath = true;
			if (players[playerIndex]->hasPath() == true) {
				players[playerIndex]->deletePath();
			}
			finishedPathfinding->unlock();
			pathFindingThread = new std::thread(&startPathfindingThread, goalX, goalY, playerIndex, this);
		}
		else {
			finishedPathfinding->unlock();
		}
	}


	void Pathfinding::startPathFinding(int goalX, int goalY, int playerIndex) {

		int* xPath = nullptr;
		int* yPath = nullptr;
		int pathlenght = 0;

		int mouseRow = goalY;
		int mouseCol = goalX;

		Player* player = players[playerIndex];
		g->enableObjectBounds(player->getRow(), player->getCol(), player->getWidth(), player->getHeight());

		//Cant do that in the other thread while pathfinding so we needa do it here
		for (int i = 0; i < playerCount; i++) {
			if (i != playerIndex) {
				Player* cPlayer = players[i];
				g->disableObjectBounds(cPlayer->getRow(), cPlayer->getCol(), cPlayer->getWidth(), cPlayer->getHeight());
			}
		}

		Algorithm::findPath(&xPath, &yPath, &pathlenght, g, player->getRow(), player->getCol(), mouseRow, mouseCol);
		//reverse accuracy simplification
		for (int i = 0; i < pathlenght; i++) {
			xPath[i] /= pathfindingAccuracy;
			yPath[i] /= pathfindingAccuracy;
		}

		finishedPathfinding->lock();
		player->givePath(xPath, yPath, pathlenght);
		findingPath = false;
		finishedPathfinding->unlock();

		g->disableObjectBounds(player->getRow(), player->getCol(), player->getWidth(), player->getHeight());
	}


	void Pathfinding::disableArea(int row, int col, int width, int height) {
		g->disableObjectBounds(row, col, width, height);
	}

	void Pathfinding::enableArea(int row, int col, int width, int height) {
		g->enableObjectBounds(row, col, width, height);
	}
