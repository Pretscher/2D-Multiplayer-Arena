#pragma once
#include "iostream" 
using namespace std;
#include <vector>
#include <thread>
#include <mutex>

class Player;
class Terrain;
class Graph;
class Pathfinding {

public:
	Pathfinding();
	unique_ptr<vector<int>> newGoalYs;
	unique_ptr<vector<int>> newGoalXs;

	void startPathFinding();
	void findPath(int goalX, int goalY, int playerIndex);

	void enablePlayer(int i_playerIndex, bool disableOthers);
	void disablePlayer(int i_playerIndex);

	void update();
	void setPlayerIndex(int playerI) {
		myPlayerIndex = playerI;
	}

	shared_ptr<mutex> getPathfingingMutex() {
		return pfMtx;
	}

	shared_ptr<Graph> getGraph() {
		return g;
	}

private:
	shared_ptr<Graph> g;
	int cgoalX;
	int cgoalY;
	int cPlayerIndex;
	bool newPathFinding;

	bool** colisionGrid;

	thread* pathFindingThread;
	shared_ptr<mutex> pfMtx;
	bool findingPath;
	shared_ptr<vector<shared_ptr<Player>>> players;
	int playerCount;
	float pathfindingAccuracy;//the higher the less accuracy (1 means every pixel is considered)

	bool sameClick = false;//dont do two pathfindings on the same click

	vector<int> goalXToFind;
	vector<int> goalYToFind;
	vector<int> indicesToFind;
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