#include "Eventhandling.hpp"
#include "Renderer.hpp"
#include "Player.hpp"
#include "aStarAlgorithm/Graph.hpp"
#include "aStarAlgorithm/Algorithm.hpp"

bool** collisionGrid;
Player* player;

int rows, cols;
float pathfindingAccuracy;
void eventhandling::init() {
	rows = 1080;
	cols = 1920;
	pathfindingAccuracy = 0.1f;
	//rows and cols are stretched to full screen anyway. Its just accuracy of rendering 
	//and relative coords on which you can orient your drawing. Also makes drawing a rect
	//and stuff easy because width can equal height to make it have sides of the same lenght.
	Renderer::initGrid(rows, cols);

	int pathfindingRows = rows * pathfindingAccuracy;
	int pathfindingCols = cols * pathfindingAccuracy;
	collisionGrid = new bool*[pathfindingRows];
	for (int y = 0; y < pathfindingRows; y++) {
		collisionGrid[y] = new bool[pathfindingCols];
		for (int x = 0; x < pathfindingCols; x++) {
			collisionGrid[y][x] = true;
		}
	}
	player = new Player(cols / 2, rows / 2, 1.0f);
}

bool sameClick = false;
void eventhandling::eventloop() {
	if (sf::Mouse::isButtonPressed(sf::Mouse::Right) == false) sameClick = false;
	//if rightclick is pressed, find path from player to position of click
	if (sf::Mouse::isButtonPressed(sf::Mouse::Right) == true && sameClick == false) {
		sameClick = true;
		Graph* g = new Graph(rows * pathfindingAccuracy, cols * pathfindingAccuracy);
		g->generateWorldGraph(collisionGrid);
		int* xPath = nullptr;
		int* yPath = nullptr;
		int pathlenght = 0;

		int mouseX, mouseY;
		Renderer::getMousePos(&mouseX, &mouseY);//writes mouse coords into mouseX, mouseY
		int mouseRow = mouseY * pathfindingAccuracy;
		int mouseCol = mouseX * pathfindingAccuracy;

		Algorithm::findPath(&xPath, &yPath, &pathlenght, g, player->getRow() * pathfindingAccuracy, player->getCol() * pathfindingAccuracy, mouseRow, mouseCol);
		
		for (int i = 0; i < pathlenght; i++) {
			xPath[i] /= pathfindingAccuracy;
			yPath[i] /= pathfindingAccuracy;
		}
		player->givePath(xPath, yPath, pathlenght);
		delete g;
	}
	player->move();
}

void eventhandling::drawingloop() {
	Renderer::drawRect(player->getRow(), player->getCol(), 100, 100, sf::Color(255, 0, 0, 255));
}