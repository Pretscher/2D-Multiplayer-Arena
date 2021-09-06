#pragma once
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

class Player;
class Terrain;
class Graph;
class Pathfinding {

public:
	Pathfinding(int worldRows, int worldCols, Terrain* terrain, Player** i_players, int i_playerCount);
	std::vector<int>* newGoalRows = new std::vector<int>();
	std::vector<int>* newGoalCols = new std::vector<int>();

	void disableArea(int row, int col, int width, int height);
	void enableArea(int row, int col, int width, int height);
	void startPathFinding();
	void findPath(int goalX, int goalY, int playerIndex);

	void enablePlayer(int i_playerIndex);
	void disablePlayer(int i_playerIndex);

	void update();
	void setPlayerIndex(int playerI) {
		myPlayerIndex = playerI;
	}

private:

	int cGoalX;
	int cGoalY;
	int cPlayerIndex;
	bool newPathFinding;

	bool** collisionGrid;

	std::thread* pathFindingThread;
	bool findingPath;
	std::mutex* finishedPathfinding;
	Player** players;
	int playerCount;
	float pathfindingAccuracy;//the higher the less accuracy (1 means every pixel is considered)
	Graph* g;
	bool sameClick = false;//dont do two pathfindings on the same click

	std::vector<int>* goalColToFind;
	std::vector<int>* goalRowToFind;
	std::vector<int>* indicesToFind;
	int myPlayerIndex = 0;

	void playerInteraction(int movedPlayerIndex);
	void workThroughPathfindingQueue();
	int wRows;
	int wCols;
	void pathFindingOnClick();
	void moveObjects();

};