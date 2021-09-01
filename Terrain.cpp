#include "Terrain.hpp"

sf::Texture wall;
sf::Texture ground;

Terrain::Terrain() {
	objectsRow = new std::vector<Rect*>();
	wall = Renderer::loadTexture("Textures/cobble.jpg", true);
	ground = Renderer::loadTexture("Textures/dirt.jpg", true);
}

void Terrain::addRect(int row, int col, int width, int height) {
	objectsRow->push_back(new Rect(row, col, width, height));
}

void Terrain::draw() {

	Renderer::drawRectWithTexture(0, 0, Renderer::getWorldCols(), Renderer::getWorldRows(), ground, true);
	for (unsigned int i = 0; i < objectsRow->size(); i++) {
		Rect* rect = this->objectsRow->at(i);
		Renderer::drawRectWithTexture(rect->getRow(), rect->getCol(), rect->getWidth(), rect->getHeight(), wall, false);
	}

}

void Terrain::addCollidablesToGrid(bool** grid, float pathfindingAccuracy, int playerWidth, int playerHeight) {
	for (int i = 0; i < objectsRow->size(); i++) {
		Rect* rect = this->objectsRow->at(i);
		for (int y = (rect->getRow() - playerHeight + (1 / pathfindingAccuracy)) * pathfindingAccuracy; y < (rect->getRow() + rect->getHeight()) * pathfindingAccuracy; y++) {
			for (int x = (rect->getCol() - playerWidth + (1 / pathfindingAccuracy)) * pathfindingAccuracy; x < (rect->getCol() + rect->getWidth()) * pathfindingAccuracy; x++) {
				grid[y][x] = false;
			}
		}
	}
}

std::vector<Rect*>* Terrain::getCollidables() {
	return objectsRow;
}