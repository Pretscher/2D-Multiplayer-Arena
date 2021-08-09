#pragma once

class Terrain;
class WorldHandling {
public:
	WorldHandling();
	void update();
	void draw();
	void hardCodeTerrain();

	inline int getWorldRows() {
		return worldRows;
	}
	inline int getWorldCols() {
		return worldCols;
	}
	inline int getFrameRows() {
		return frameRows;
	}
	inline int getFrameCols() {
		return frameCols;
	}
	inline Terrain* getTerrain() {
		return terrain;
	}

private:
	int worldRows;
	int worldCols;
	Terrain* terrain;//contains every non-moving object with collision

	//Viewspace: value of 0 means left/top, limit value (from viewSpaceLimits) means right/bottom
	int* cViewSpace;//current viewspace position. [0] is row (bot to top), [1] is col (left to right) 
	int* viewSpaceLimits;//maximum viewspace.  [0] = left, [1] = right, [2] = top, [3] = bottom

	int frameRows, frameCols;//rows and cols you can see at a time, viewspace limits need to be added for worldSize
};