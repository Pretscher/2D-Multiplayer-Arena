#include "Terrain.hpp"
#include "GlobalRecources.hpp"

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

	Renderer::drawRectWithTexture(0, 0, Renderer::getWorldCols() * 2, Renderer::getWorldRows() * 2, ground, true);
	for (unsigned int i = 0; i < objectsRow->size(); i++) {
		Rect* rect = this->objectsRow->at(i);
		Renderer::drawRectWithTexture(rect->getRow(), rect->getCol(), rect->getWidth(), rect->getHeight(), wall, false);
	}

}

void Terrain::addCollidablesToGrid(bool** grid, float pathfindingAccuracy, int playerWidth, int playerHeight) {
	for (int i = 0; i < objectsRow->size(); i++) {
		Rect* rect = this->objectsRow->at(i);

		int startY = ((float)rect->getRow() - playerHeight + (1.0f / pathfindingAccuracy)) * pathfindingAccuracy;
		if (startY < 0) startY = 0;
		int startX = ((float)rect->getCol() - playerWidth + (1.0f / pathfindingAccuracy)) * pathfindingAccuracy;
		if (startX < 0) startX = 0;

		int endY = ((float)rect->getRow() + rect->getHeight()) * pathfindingAccuracy;
		if (endY >= GlobalRecources::worldRows) endY = GlobalRecources::worldRows - 1;
		int endX = ((float) rect->getCol() + rect->getWidth()) * pathfindingAccuracy;
		if (endX >= GlobalRecources::worldCols) endX = GlobalRecources::worldCols - 1;

		for (int y = startY; y < endY; y++) {
			for (int x = startX; x < endX; x++) {
				grid[y][x] = false;
			}
		}
	}
}

std::vector<Rect*>* Terrain::getCollidables() {
	return objectsRow;
}