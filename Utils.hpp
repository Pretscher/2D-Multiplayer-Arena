#pragma once

namespace Utils {
	bool collisionCoordsRect(float rX, float rY, float rW, float rH, float pX, float pY);
	/**
	* @param top left coords of and circle
	**/
	bool collisionCoordsCircle(float cX, float cY, float cR, float pX, float pY);
	/**
	* @param top left coords of rect and circle, colPointDist is accuracy of detection
	**/
	bool collisionRectCircle(int aCol, int aRow, int aW, int aH, int cCol, int cRow, int cR, int colPointDist);
	bool collisionRects(float aX, float aY, float aW, float aH, float bX, float bY, float bW, float bH, float colPointDist);
	float RandomNumber(float Min, float Max);
	bool compareFloats(float a, float b, float accuracy);
	float calcDist1D(float y1, float y2);

	bool collisionRectCircleOnlyOutline(int aCol, int aRow, int aW, int aH, int cCol, int cRow, int cR);
};