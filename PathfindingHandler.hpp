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
	Pathfinding(int worldYs, int worldXs, Terrain* terrain, Player** i_players, int i_playerCount);
	std::vector<int>* newGoalYs = new std::vector<int>();
	std::vector<int>* newGoalXs = new std::vector<int>();

	void disableArea(int y, int x, int width, int height);
	void enableArea(int y, int x, int width, int height);
	void startPathFinding();
	void findPath(int goalX, int goalY, int playerIndex);

	void enablePlayer(int i_playerIndex, bool disableOthers);
	void disablePlayer(int i_playerIndex);

	void update();
	void setPlayerIndex(int playerI) {
		myPlayerIndex = playerI;
	}

	std::mutex* getPathfingingMutex() {
		return pfMtx;
	}

	Graph* getGraph() {
		return g;
	}

private:
	Graph* g;
	int cgoalX;
	int cgoalY;
	int cPlayerIndex;
	bool newPathFinding;

	bool** colisionGrid;

	std::thread* pathFindingThread;
	std::mutex* pfMtx;
	bool findingPath;
	Player** players;
	int playerCount;
	float pathfindingAccuracy;//the higher the less accuracy (1 means every pixel is considered)

	bool sameClick = false;//dont do two pathfindings on the same click

	std::vector<int>* goalXToFind;
	std::vector<int>* goalYToFind;
	std::vector<int>* indicesToFind;
	int myPlayerIndex = 0;

	void playerInteraction(int movedPlayerIndex);
	void workThroughPathfindingQueue();
	int wYs;
	int wXs;
	void pathFindingOnClick();
	void moveObjects();

	void setNewPathfinding(bool i_newPf);
	bool getNewPathfinding();
	void setFindingPath(bool i_newPf);

	//thread safety: this var should only be accessed through this getter, but you dont have to handle the mutex yourself
	bool getFindingPath();

};