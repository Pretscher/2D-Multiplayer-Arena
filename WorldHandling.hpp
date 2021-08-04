#include "Renderer.hpp"
#include "Terrain.hpp"

class WorldHandling {
public:
	WorldHandling() {
		viewSpaceLimits = new int[4];
		viewSpaceLimits[0] = 0;//left
		viewSpaceLimits[1] = 2000;//right
		viewSpaceLimits[2] = 0;//top
		viewSpaceLimits[3] = 2000;//bot
		cViewSpace = new int[2];
		cViewSpace[0] = 0;//row (top to bot)
		cViewSpace[1] = 0;//col (left to right)



		//hardcoded rows and cols
		frameRows = 1080;
		frameCols = 1920;
		worldRows = frameRows + viewSpaceLimits[3];
		worldCols = frameCols + viewSpaceLimits[1];


		Renderer::initGrid(frameRows, frameCols);
		Renderer::linkViewSpace(cViewSpace, viewSpaceLimits);
		terrain = new Terrain();
		hardCodeTerrain();
	}

	void update() {
		Renderer::updateViewSpace();//move view space if mouse on edge of window
	}

	void draw() {
		terrain->draw();
	}

	void hardCodeTerrain() {
		terrain->addRect(1000, 1000, 500, 200);
		terrain->addRect(200, 200, 500, 200);
		terrain->addRect(1000, 1000, 500, 200);
	}



	int worldRows;
	int worldCols;
	Terrain* terrain;//contains every non-moving object with collision

	//Viewspace: value of 0 means left/top, limit value (from viewSpaceLimits) means right/bottom
	int* cViewSpace;//current viewspace position. [0] is row (bot to top), [1] is col (left to right) 
	int* viewSpaceLimits;//maximum viewspace.  [0] = left, [1] = right, [2] = top, [3] = bottom

	int frameRows, frameCols;//rows and cols you can see at a time, viewspace limits need to be added for worldSize
};