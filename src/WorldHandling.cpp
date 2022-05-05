#include "WorldHandling.hpp"	
#include "iostream" 
using namespace std;

#include "Renderer.hpp"
#include "Terrain.hpp"
#include "GlobalRecources.hpp"
WorldHandling::WorldHandling(int frameWidth, int frameHeight, int viewspaceLimitsX, int viewspaceLimitsY) {
	//hardcoded ys and xs
	worldHeight = frameHeight + viewspaceLimitsY;
	worldWidth = frameWidth + viewspaceLimitsX;

	viewSpaceLimits = shared_ptr<const int[]>(new int[4]{0, viewspaceLimitsX, 0, viewspaceLimitsY});
	cViewSpace = shared_ptr<int[]>(new int[2]);
	cViewSpace[0] = 0;//y (top to bot)
	cViewSpace[1] = 0;//x (left to right)

	Renderer::linkViewSpace(cViewSpace, viewSpaceLimits);
	terrain = shared_ptr<Terrain>(new Terrain());
	hardCodeTerrain();

	GlobalRecources::terrain = terrain;
	GlobalRecources::frameWidth = frameWidth;
	GlobalRecources::frameHeight = frameHeight;
	GlobalRecources::worldWidth = worldWidth;
	GlobalRecources::worldHeight = worldHeight;
}

void WorldHandling::update() {
	Renderer::updateViewSpace();//move view space if mouse on edge of window
}

void WorldHandling::draw() {
	terrain->draw();
}

void WorldHandling::hardCodeTerrain() {
	terrain->addRect(0, 0, worldWidth, 50);
	terrain->addRect(0, 0, 50, worldHeight);
	terrain->addRect(0, worldWidth - 50, 50, worldHeight);
	terrain->addRect(worldHeight - 70, 0, worldWidth, 50);

	terrain->addRect(300, 0, 300, 100);
	terrain->addRect(300, 300, 100, 300);

	terrain->addRect(1000, 1000, 500, 500);
}
