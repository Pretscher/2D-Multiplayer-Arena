#pragma once
#include "Renderer.hpp"

class Rect {
public:
	Rect(int row, int col, int width, int height) {
		this->row = row;
		this->col = col;
		this->width = width;
		this->height = height;
	}

private:
	int row;
	int col;
	int width;
	int height;

public:
    inline int getRow() { return row; }
	inline void setRow(int row) { this->row = row; }

	inline int getCol() { return col; }
	inline void setCol(int col) { this->col = col; }

	inline int getWidth() { return width; }
	inline void setWidth(int width) { this->width = width; }

	inline int getHeight() { return height; }
	inline void setHeight(int height) { this->height = height; }

};

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
			Renderer::drawRect(rect->getRow(), rect->getCol(), rect->getWidth(), rect->getHeight(), sf::Color(100, 100, 100, 255));
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

private:
	std::vector<Rect*>* objectsRow;
};