#pragma once
#include "iostream" 
#include <memory>
using namespace std;
class Terrain;
class WorldHandling {
public:
	WorldHandling(int frameWidth, int frameHeight, int viewspaceLimitsX, int viespaceLimitsY);
	void update();
	void draw();
	void hardCodeTerrain();

	inline int getWorldYs() {
		return worldHeight;
	}
	inline int getWorldXs() {
		return worldWidth;
	}
	inline int getFrameYs() {
		return frameHeight;
	}
	inline int getFrameXs() {
		return frameWidth;
	}
	inline shared_ptr<Terrain> getTerrain() {
		return terrain;
	}

private:
	int worldHeight;
	int worldWidth;
	shared_ptr<Terrain> terrain;//contains every non-moving object with colision

	//Viewspace: value of 0 means left/top, limit value (from viewSpaceLimits) means right/bottom
	shared_ptr<int[]> cViewSpace;//current viewspace position.[0] is y (bot to top),[1] is x (left to right) 
	shared_ptr<const int[]> viewSpaceLimits;//maximum viewspace. [0] = left,[1] = right,[2] = top,[3] = bottom

	int frameHeight, frameWidth;//ys and xs you can see at a time, viewspace limits need to be added for worldSize
};