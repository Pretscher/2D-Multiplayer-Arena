#pragma once

namespace Utils {
	bool colisionCoordsRect(float rectX, float rectY, float rectWidth, float rectHeight, float pointX, float pointY);
	/**
	* @param top left coords of and circle
	**/
	bool colisionCoordsCircle(float circleX, float circleY, float circleRadius, float pointX, float pointY);
	/**
	* @param top left coords of rect and circle, xPointDist is accuracy of detection
	**/
	bool colisionRectCircle(int rectX, int rectY, int rectWidth, int rectHeight, int circleX, int circleY, int circleRadius, int xPointDist);
	bool colisionRectCircleOnlyOutline(int rectX, int rectY, int rectWidth, int rectHeight, int circleX, int circleY, int circleRadius);

	bool colisionRects(float rect1X, float rect1Y, float rect1Width, float rect1Height, float rect2X, float rect2Y, float rect2Width, float rect2Height, float xPointDist);
	float RandomNumber(float Min, float Max);
	bool compareFloats(float a, float b, float accuracy);
	float calcDist1D(float a, float b);
	float calcDist2D(float x1, float x2, float y1, float y2);

};