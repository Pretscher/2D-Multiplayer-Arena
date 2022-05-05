#include "Terrain.hpp"
#include "GlobalRecources.hpp"
#include "iostream" 
using namespace std;

sf::Texture wall;
sf::Texture ground;

Terrain::Terrain() {
	objects = unique_ptr<vector<Rect>>(new vector<Rect>());
	wall = Renderer::loadTexture("myRecources/Textures/cobble.jpg", true);
	ground = Renderer::loadTexture("myRecources/Textures/dirt.jpg", true);
}

void Terrain::addRect(int y, int x, int width, int height) {
	objects->push_back(Rect(y, x, width, height));
}

void Terrain::draw() {

	Renderer::drawRectWithTexture(0, 0, 1920, 1080, ground, true);
	for (unsigned int i = 0; i < objects->size(); i++) {
		const Rect& rect = this->objects->at(i);
		Renderer::drawRectWithTexture(rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight(), wall, false);
	}

}

void Terrain::addCollidablesToGrid(bool** grid, float pathfindingAccuracy, int playerWidth, int playerHeight) {
	for (int i = 0; i < objects->size(); i++) {
		const Rect& rect = this->objects->at(i);

		int startY = ((float)rect.getY() - playerHeight + (1.0f / pathfindingAccuracy)) * pathfindingAccuracy;
		if (startY < 0) startY = 0;
		int startX = ((float)rect.getX() - playerWidth + (1.0f / pathfindingAccuracy)) * pathfindingAccuracy;
		if (startX < 0) startX = 0;

		int endY = ((float)rect.getY() + rect.getHeight()) * pathfindingAccuracy;
		if (endY >= GlobalRecources::worldHeight) endY = GlobalRecources::worldHeight - 1;
		int endX = ((float) rect.getX() + rect.getWidth()) * pathfindingAccuracy;
		if (endX >= GlobalRecources::worldWidth) endX = GlobalRecources::worldWidth - 1;

		for (int y = startY; y < endY; y++) {
			for (int x = startX; x < endX; x++) {
				grid[y][x] = false;
			}
		}
	}
}

const shared_ptr<vector<Rect>> Terrain::getCollidables() {
	return objects;
}