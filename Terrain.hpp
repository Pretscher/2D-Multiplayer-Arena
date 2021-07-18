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

	void addCollidablesToGrid(bool** grid, int startRow, int startCol, int maxRow, int maxCol) {
		for (int i = 0; i < objectsRow->size(); i++) {
			Rect* rect = this->objectsRow->at(i);
			//check if rect is in bounds of given coordinates (view coordinates)
			if (rect->getCol() + rect->getWidth() < startCol
				|| rect->getCol() > maxCol
				|| rect->getRow() + rect->getHeight() < startRow
				|| rect->getRow() > maxRow) {
				//not in bounds, do nothing for this rect
			}
			else {
				for (int y = rect->getRow(); y < rect->getRow() + rect->getHeight(); y++) {
					for (int x = rect->getCol(); x < rect->getCol() + rect->getWidth(); x++) {
						grid[y][x] = false;
					}
				}
			}
		}
	}

private:
	std::vector<Rect*>* objectsRow;
};