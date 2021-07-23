#include "Terrain.hpp"

Terrain::Terrain() {
	objectsRow = new std::vector<Rect*>();
}

void Terrain::addRect(int row, int col, int width, int height) {
	objectsRow->push_back(new Rect(row, col, width, height));
}

void Terrain::draw() {
	for (unsigned int i = 0; i < objectsRow->size(); i++) {
		Rect* rect = this->objectsRow->at(i);
		Renderer::drawRect(rect->getRow(), rect->getCol(), rect->getWidth(), rect->getHeight(), sf::Color(100, 100, 100, 100));
	}
}

void Terrain::addCollidablesToGrid(bool** grid, int pathfindingAccuracy, int playerWidth, int playerHeight) {
	for (int i = 0; i < objectsRow->size(); i++) {
		Rect* rect = this->objectsRow->at(i);
		for (int y = (rect->getRow() - playerHeight + pathfindingAccuracy) / pathfindingAccuracy; y < (rect->getRow() + rect->getHeight()) / pathfindingAccuracy; y++) {
			for (int x = (rect->getCol() - playerWidth + pathfindingAccuracy) / pathfindingAccuracy; x < (rect->getCol() + rect->getWidth()) / pathfindingAccuracy; x++) {
				grid[y][x] = false;
			}
		}
	}
}

std::vector<Rect*>* Terrain::getCollidables() {
	return objectsRow;
}