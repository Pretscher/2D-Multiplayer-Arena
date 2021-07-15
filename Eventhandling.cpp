#include "Eventhandling.hpp"
#include "Renderer.hpp"
#include "Player.hpp"

bool** collisionGrid;
Player* player;

int rows, cols;
void eventhandling::init() {
	rows = 1920;
	cols = 1080;
	//rows and cols are stretched to full screen anyway. Its just accuracy of rendering 
	//and relative coords on which you can orient your drawing. Also makes drawing a rect
	//and stuff easy because width can equal height to make it have sides of the same lenght.
	Renderer::initGrid(rows, cols);


	collisionGrid = new bool*[rows];
	for (int y = 0; y < rows; y++) {
		collisionGrid[y] = new bool[cols];
		for (int x = 0; x < cols; x++) {
			collisionGrid[y][x] = false;
		}
	}
	player = new Player(cols / 2, rows / 2, 0.01f);
}

void eventhandling::eventloop() {
	
}

void eventhandling::drawingloop() {
	Renderer::drawRect(player->getY(), player->getX(), 100, 100, sf::Color(255, 0, 0, 255));
}