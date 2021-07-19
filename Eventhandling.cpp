#include "Eventhandling.hpp"
#include "Renderer.hpp"
#include "Player.hpp"
#include "aStarAlgorithm/Graph.hpp"
#include "aStarAlgorithm/Algorithm.hpp"
#include "Terrain.hpp"

Player* player;//movable player
Terrain* terrain;//contains every non-moving object with collision

//Viewspace: value of 0 means left/top, limit value (from viewSpaceLimits) means right/bottom
int* cViewSpace;//current viewspace position. [0] is row (bot to top), [1] is col (left to right) 
int* viewSpaceLimits;//maximum viewspace.  [0] = left, [1] = right, [2] = top, [3] = bottom

int rows, cols;//rows and cols you can see at a time, viewspace limits need to be added for worldSize
int pathfindingAccuracy;//the higher the less accuracy (1 means every pixel is considered)

//double array for every visitable (pathfindingaccuracy factored in) pixel, 
//if it can be used for pathfinding true, else false
bool** collisionGrid;

//forward declarations
void pathFindingOnClick();
void pathFindingInit();
void hardCodeTerrain();

void eventhandling::init() {
	player = new Player(cols / 2, rows / 2, 100, 100, 1.0f);

	terrain = new Terrain();
	hardCodeTerrain();

	pathFindingInit();
}

void eventhandling::eventloop() {
	pathFindingOnClick();//right click => find path to right clicked spot and give it to player
	player->move();//if he has a path, he moves on this path
	Renderer::updateViewSpace(cViewSpace, viewSpaceLimits, rows, cols);//move view space if mouse on edge of window
}

void eventhandling::drawingloop() {
	terrain->draw();
	Renderer::drawRect(player->getRow(), player->getCol(), player->getDrawWidth(), player->getDrawHeight(), sf::Color(255, 0, 0, 255));//draw Player
}



//Pathfinding--------------------------------------------------------------------------------------------

//rows and cols of one visible window + viewspace limit coords = max coords we need for pathfinding
int worldRows;
int worldCols;
void pathFindingInit() {
	viewSpaceLimits = new int[4];
	viewSpaceLimits[0] = 0;//left
	viewSpaceLimits[1] = 2000;//right
	viewSpaceLimits[2] = 0;//top
	viewSpaceLimits[3] = 2000;//bot
	cViewSpace = new int[2];
	cViewSpace[0] = 0;//row (top to bot)
	cViewSpace[1] = 0;//col (left to right)

	//hardcoded rows and cols
	rows = 1080;
	cols = 1920;
	worldRows = rows + viewSpaceLimits[3];
	worldCols = cols + viewSpaceLimits[1];

	pathfindingAccuracy = 10;
	//rows and cols are stretched to full screen anyway. Its just accuracy of rendering 
	//and relative coords on which you can orient your drawing. Also makes drawing a rect
	//and stuff easy because width can equal height to make it have sides of the same lenght.
	Renderer::initGrid(rows, cols);

	int pathfindingRows = worldRows / pathfindingAccuracy;//max rows for pathfinding
	int pathfindingCols = worldCols / pathfindingAccuracy;//max cols for pathfinding
	collisionGrid = new bool* [pathfindingRows];
	for (int y = 0; y < pathfindingRows; y++) {
		collisionGrid[y] = new bool[pathfindingCols];
		for (int x = 0; x < pathfindingCols; x++) {
			collisionGrid[y][x] = true;
		}
	}
	terrain->addCollidablesToGrid(collisionGrid, pathfindingAccuracy, player->getDrawWidth(), player->getDrawHeight());
}

bool sameClick = false;//dont do two pathfindings on the same click
void pathFindingOnClick() {
	if (sf::Mouse::isButtonPressed(sf::Mouse::Right) == false){
		sameClick = false;
	}

	//if rightclick is pressed, find path from player to position of click
	if (sf::Mouse::isButtonPressed(sf::Mouse::Right) == true && sameClick == false) {
		sameClick = true;
		Graph* g = new Graph(worldRows / pathfindingAccuracy, worldCols / pathfindingAccuracy);
		g->generateWorldGraph(collisionGrid);
		int* xPath = nullptr;
		int* yPath = nullptr;
		int pathlenght = 0;

		int mouseX = -1, mouseY = -1;
		Renderer::getMousePos(&mouseX, &mouseY);//writes mouse coords into mouseX, mouseY
		if (mouseX != -1) {//stays at -1 if click is outside of window
			int mouseRow = (mouseY + cViewSpace[0]) / pathfindingAccuracy;
			int mouseCol = (mouseX + cViewSpace[1]) / pathfindingAccuracy;

			Algorithm::findPath(&xPath, &yPath, &pathlenght, g, player->getRow() / pathfindingAccuracy, player->getCol() / pathfindingAccuracy, mouseRow, mouseCol);
			//reverse accuracy simplification
			for (int i = 0; i < pathlenght; i++) {
				xPath[i] *= pathfindingAccuracy;
				yPath[i] *= pathfindingAccuracy;
			}
			player->givePath(xPath, yPath, pathlenght);
			delete g;
		}
	}
}

//Game Object initialization

void hardCodeTerrain() {
	terrain->addRect(1000, 1000, 500, 200);
}