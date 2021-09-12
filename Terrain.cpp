#include "Terrain.hpp"
#include "GlobalRecources.hpp"

sf::Texture wall;
sf::Texture ground;

Terrain::Terrain() {
	objectsY = new std::vector<Rect*>();
	wall = Renderer::loadTexture("Textures/cobble.jpg", true);
	ground = Renderer::loadTexture("Textures/dirt.jpg", true);
}

void Terrain::addRect(int y, int x, int width, int height) {
	objectsY->push_back(new Rect(y, x, width, height));
}

void Terrain::draw() {

	Renderer::drawRectWithTexture(0, 0, 1920, 1080, ground, true);
	for (unsigned int i = 0; i < objectsY->size(); i++) {
		Rect* rect = this->objectsY->at(i);
		Renderer::drawRectWithTexture(rect->getX(), rect->getY(), rect->getWidth(), rect->getHeight(), wall, false);
	}

}

void Terrain::addXlidablesToGrid(bool** grid, float pathfindingAccuracy, int playerWidth, int playerHeight) {
	for (int i = 0; i < objectsY->size(); i++) {
		Rect* rect = this->objectsY->at(i);

		int startY = ((float)rect->getY() - playerHeight + (1.0f / pathfindingAccuracy)) * pathfindingAccuracy;
		if (startY < 0) startY = 0;
		int startX = ((float)rect->getX() - playerWidth + (1.0f / pathfindingAccuracy)) * pathfindingAccuracy;
		if (startX < 0) startX = 0;

		int endY = ((float)rect->getY() + rect->getHeight()) * pathfindingAccuracy;
		if (endY >= GlobalRecources::worldYs) endY = GlobalRecources::worldYs - 1;
		int endX = ((float) rect->getX() + rect->getWidth()) * pathfindingAccuracy;
		if (endX >= GlobalRecources::worldXs) endX = GlobalRecources::worldXs - 1;

		for (int y = startY; y < endY; y++) {
			for (int x = startX; x < endX; x++) {
				grid[y][x] = false;
			}
		}
	}
}

std::vector<Rect*>* Terrain::getXlidables() {
	return objectsY;
}