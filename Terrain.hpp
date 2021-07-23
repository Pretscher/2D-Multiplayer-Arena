#pragma once
#include "Renderer.hpp"
#include "Rect.hpp"
#include <iostream>
class Terrain {
public:
	Terrain();
	void addRect(int row, int col, int width, int height);
	void draw();
	void addCollidablesToGrid(bool** grid, int pathfindingAccuracy, int playerWidth, int playerHeight);
	std::vector<Rect*>* getCollidables();

private:
	std::vector<Rect*>* objectsRow;
};