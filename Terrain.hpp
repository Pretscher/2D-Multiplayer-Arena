#pragma once
#include "Renderer.hpp"
#include "Rect.hpp"
#include "iostream" 
using namespace std;

class Terrain {
public:
	Terrain();
	void addRect(int y, int x, int width, int height);
	void draw();
	void addCollidablesToGrid(bool** grid, float pathfindingAccuracy, int playerWidth, int playerHeight);
	vector<Rect*>* getCollidables();

private:
	vector<Rect*>* objectsY;
};