#include "Eventhandling.hpp"
#include "Renderer.hpp"
#include "Player.hpp"
#include "aStarAlgorithm/Graph.hpp"
#include "aStarAlgorithm/Algorithm.hpp"
#include "Terrain.hpp"

Player* player;
Terrain* terrain;

int* cViewSpace;
int* viewSpaceLimits;

int rows, cols;
int pathfindingAccuracy;
bool** collisionGrid;
void pathFindingOnClick();
void pathFindingInit();
void hardCodeTerrain();

void eventhandling::init() {
	player = new Player(cols / 2, rows / 2, 1.0f);

	terrain = new Terrain();
	hardCodeTerrain();

	pathFindingInit();
}

void eventhandling::eventloop() {
	pathFindingOnClick();
	player->move();
	Renderer::updateViewSpace(cViewSpace, viewSpaceLimits, rows, cols);
}

void eventhandling::drawingloop() {
	terrain->draw();
	Renderer::drawRect(player->getRow(), player->getCol(), 100, 100, sf::Color(255, 0, 0, 255));
}



//Pathfinfing--------------------------------------------------------------------------------------------

void pathFindingInit() {
	viewSpaceLimits = new int[4];
	viewSpaceLimits[0] = 0;//left
	viewSpaceLimits[1] = 2000;//right
	viewSpaceLimits[2] = 0;//top
	viewSpaceLimits[3] = 2000;//bot
	cViewSpace = new int[2];
	cViewSpace[0] = 1000;//row (top to bot)
	cViewSpace[1] = 1000;//col (left to right)

	rows = 1080;
	cols = 1920;
	pathfindingAccuracy = 10;
	//rows and cols are stretched to full screen anyway. Its just accuracy of rendering 
	//and relative coords on which you can orient your drawing. Also makes drawing a rect
	//and stuff easy because width can equal height to make it have sides of the same lenght.
	Renderer::initGrid(rows, cols);

	int pathfindingRows = rows * pathfindingAccuracy;
	int pathfindingCols = cols * pathfindingAccuracy;
	collisionGrid = new bool* [pathfindingRows];
	for (int y = 0; y < pathfindingRows; y++) {
		collisionGrid[y] = new bool[pathfindingCols];
		for (int x = 0; x < pathfindingCols; x++) {
			collisionGrid[y][x] = true;
		}
	}
	terrain->addCollidablesToGrid(collisionGrid, cViewSpace[0], cViewSpace[1], rows - cViewSpace[0], cols - cViewSpace[1]);
}

bool sameClick = false;
void pathFindingOnClick() {
	if (sf::Mouse::isButtonPressed(sf::Mouse::Right) == false) sameClick = false;
	//if rightclick is pressed, find path from player to position of click
	if (sf::Mouse::isButtonPressed(sf::Mouse::Right) == true && sameClick == false) {
		sameClick = true;
		Graph* g = new Graph(rows / pathfindingAccuracy, cols / pathfindingAccuracy);
		g->generateWorldGraph(collisionGrid);
		int* xPath = nullptr;
		int* yPath = nullptr;
		int pathlenght = 0;

		int mouseX = -1, mouseY = -1;
		Renderer::getMousePos(&mouseX, &mouseY);//writes mouse coords into mouseX, mouseY
		if (mouseX != -1) {//stays at -1 if click is outside of window
			int mouseRow = mouseY / pathfindingAccuracy;
			int mouseCol = mouseX / pathfindingAccuracy;

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

}