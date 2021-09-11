#include "WorldHandling.hpp"	

#include "Renderer.hpp"
#include "Terrain.hpp"

WorldHandling::WorldHandling() {
	viewSpaceLimits = new int[4];
	viewSpaceLimits[0] = 0;//left
	viewSpaceLimits[1] = 2000;//right
	viewSpaceLimits[2] = 0;//top
	viewSpaceLimits[3] = 2000;//bot
	cViewSpace = new int[2];
	cViewSpace[0] = 0;//y (top to bot)
	cViewSpace[1] = 0;//x (left to right)



	//hardcoded ys and xs
	frameYs = 1080;
	frameXs = 1920;
	worldYs = frameYs + viewSpaceLimits[3];
	worldXs = frameXs + viewSpaceLimits[1];


	Renderer::linkViewSpace(cViewSpace, viewSpaceLimits);
	terrain = new Terrain();
	hardCodeTerrain();
}

void WorldHandling::update() {
	Renderer::updateViewSpace();//move view space if mouse on edge of window
}

void WorldHandling::draw() {
	terrain->draw();
}

void WorldHandling::hardCodeTerrain() {
	terrain->addRect(0, 0, worldXs, 50);
	terrain->addRect(0, 0, 50, worldYs);
	terrain->addRect(0, worldXs - 50, 50, worldYs);
	terrain->addRect(worldYs - 70, 0, worldXs, 50);

	terrain->addRect(300, 0, 300, 100);
	terrain->addRect(300, 300, 100, 300);

	terrain->addRect(1000, 1000, 500, 500);
}
