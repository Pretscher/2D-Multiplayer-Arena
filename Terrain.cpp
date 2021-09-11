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

		int startRow = ((float)rect->getRow() - playerHeight + (1.0f / pathfindingAccuracy)) * pathfindingAccuracy;
		if (startRow < 0) startRow = 0;
		int startCol = ((float)rect->getCol() - playerWidth + (1.0f / pathfindingAccuracy)) * pathfindingAccuracy;
		if (startCol < 0) startCol = 0;

		int endRow = ((float)rect->getRow() + rect->getHeight()) * pathfindingAccuracy;
		if (endRow >= GlobalRecources::worldRows) endRow = GlobalRecources::worldRows - 1;
		int endCol = ((float) rect->getCol() + rect->getWidth()) * pathfindingAccuracy;
		if (endCol >= GlobalRecources::worldCols) endCol = GlobalRecources::worldCols - 1;

		for (int row = startRow; row < endRow; row++) {
			for (int col = startCol; col < endCol; col++) {
				grid[row][col] = false;
			}
		}
	}
}

std::vector<Rect*>* Terrain::getCollidables() {
	return objectsRow;
}