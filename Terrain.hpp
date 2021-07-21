#pragma once
#include "Renderer.hpp"
#include "Rect.hpp"
class Terrain {
public:
	Terrain() {
		objectsRow = new std::vector<Rect*>();
	}

	void addRect(int row, int col, int width, int height) {
		objectsRow->push_back(new Rect(row, col, width, height));
	}

	void draw() {
		for (unsigned int i = 0; i < objectsRow->size(); i++) {
			Rect* rect = this->objectsRow->at(i);
			Renderer::drawRect(rect->getRow(), rect->getCol(), rect->getWidth(), rect->getHeight(), sf::Color(100, 100, 100, 100));
		}
	}

	void addCollidablesToGrid(bool** grid, int pathfindingAccuracy, int playerWidth, int playerHeight) {
		for (int i = 0; i < objectsRow->size(); i++) {
			Rect* rect = this->objectsRow->at(i);
			for (int y = (rect->getRow() - playerHeight + pathfindingAccuracy) / pathfindingAccuracy; y < (rect->getRow() + rect->getHeight()) / pathfindingAccuracy; y++) {
				for (int x = (rect->getCol() - playerWidth + pathfindingAccuracy) / pathfindingAccuracy; x < (rect->getCol() + rect->getWidth()) / pathfindingAccuracy; x++) {
					grid[y][x] = false;
				}
			}
		}
	}

	inline std::vector<Rect*>* getCollidables() {
		return objectsRow;
	}

private:
	std::vector<Rect*>* objectsRow;
};