#pragma once
#include <iostream>
#include <thread>
#include <mutex>

class Player;
class Terrain;
class Graph;
class Pathfinding {
private:
    bool** collisionGrid;

	std::thread* pathFindingThread;
	bool findingPath = false;
	std::mutex* finishedPathfinding;
	Player** players;
	int playerCount;
	float pathfindingAccuracy;//the higher the less accuracy (1 means every pixel is considered)
	Graph* g;
	bool sameClick = false;//dont do two pathfindings on the same click
public:
	Pathfinding(int worldRows, int worldCols, Terrain* terrain, Player** i_players, int i_playerCount);
	void pathFindingOnClick(int playerIndex);
	void findPath(int goalX, int goalY, int playerIndex);
	void startPathFinding(int goalX, int goalY, int playerIndex);
	void disableArea(int row, int col, int width, int height);
	void enableArea(int row, int col, int width, int height);

	void lockPlayer() {
		finishedPathfinding->lock();
	}
	void unlockPlayer() {
		finishedPathfinding->unlock();
	}
	bool isPlayerUseable() {
		return !findingPath;
	}
};