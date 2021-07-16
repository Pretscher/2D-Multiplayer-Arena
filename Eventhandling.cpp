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
	rows = 1920;
	cols = 1080;
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
	player = new Player(cols / 2, rows / 2, 0.01f);
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
		Algorithm::findPath(xPath, yPath, &pathlenght, g, player->getX() * pathfindingAccuracy, player->getY() * pathfindingAccuracy, mouseX * pathfindingAccuracy, mouseY * pathfindingAccuracy);
		
		delete g;
		delete[] xPath;
		delete[] yPath;
	}
}

void eventhandling::drawingloop() {
	Renderer::drawRect(player->getY(), player->getX(), 100, 100, sf::Color(255, 0, 0, 255));
}