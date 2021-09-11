#pragma once

class Terrain;
class WorldHandling {
public:
	WorldHandling();
	void update();
	void draw();
	void hardCodeTerrain();

	inline int getWorldYs() {
		return worldYs;
	}
	inline int getWorldXs() {
		return worldXs;
	}
	inline int getFrameYs() {
		return frameYs;
	}
	inline int getFrameXs() {
		return frameXs;
	}
	inline Terrain* getTerrain() {
		return terrain;
	}

private:
	int worldYs;
	int worldXs;
	Terrain* terrain;//contains every non-moving object with xlision

	//Viewspace: value of 0 means left/top, limit value (from viewSpaceLimits) means right/bottom
	int* cViewSpace;//current viewspace position.[0] is y (bot to top),[1] is x (left to right) 
	int* viewSpaceLimits;//maximum viewspace. [0] = left,[1] = right,[2] = top,[3] = bottom

	int frameYs, frameXs;//ys and xs you can see at a time, viewspace limits need to be added for worldSize
};