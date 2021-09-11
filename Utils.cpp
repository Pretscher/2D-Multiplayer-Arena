#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include "Utils.hpp"

bool Utils::xlisionCoordsRect(float rectX, float rectY, float rectWidth, float rectHeight, float pointX, float pointY) {
	if (rectX < pointX && rectX + rectWidth > pointX) {
		if (rectY < pointY && rectY + rectHeight > pointY) {
			return true;
		}
	}
	return false;
}
/**
* @param top left coords of circle and point coords
**/
bool Utils::xlisionCoordsCircle(float circleX, float circleY, float circleRadius, float pointX, float pointY) {
	float dist = sqrt((pointX - (circleX + circleRadius)) * (pointX - (circleX + circleRadius)) + (pointY - (circleY + circleRadius)) * (pointY - (circleY + circleRadius)));
	if (dist <= circleRadius) return true;
	return false;
}

/**
* @param top left coords of rect and circle, xPointDist is accuracy of detection
**/
bool Utils::xlisionRectCircle(int rectX, int rectY, int rectWidth, int rectHeight, int circleX, int circleY, int circleRadius, int xPointDist) {
	//check points of second rect for intersection with circle with given accuracy
	for (float x = rectX; x <= rectX + rectWidth; x += xPointDist) {
		for (float y = rectY; y <= rectY + rectHeight; y += xPointDist) {
			//Renderer::drrectWidthRect(y, x, 2, 2, sf::Color(255, 255, 0, 255), false);
			if (Utils::xlisionCoordsCircle(circleX, circleY, circleRadius, x, y) == true) {
				return true;
			}
		}
	}
	return false;
}

bool Utils::xlisionRectCircleOnlyOutline(int rectX, int rectY, int rectWidth, int rectHeight, int circleX, int circleY, int circleRadius) {
	//check points of second rect for intersection with circle with given accuracy
	for (float y = rectY; y <= rectY + rectHeight; y += rectHeight) {
		for (float x = rectX; x <= rectX + rectWidth; x += (circleRadius * 2 - 1)) {
			//Renderer::drrectWidthRect(y, x, 2, 2, sf::Color(255, 255, 0, 255));
			if (Utils::xlisionCoordsCircle(circleX, circleY, circleRadius, x, y) == true) {
				return true;
			}
		}
	}
	for (float x = rectX; x <= rectX + rectWidth; x += rectWidth) {
		for (float y = rectY; y <= rectY + rectHeight; y += (circleRadius * 2 - 1)) {
			//Renderer::drrectWidthRect(y, x, 2, 2, sf::Color(255, 255, 0, 255));
			if (Utils::xlisionCoordsCircle(circleX, circleY, circleRadius, x, y) == true) {
				return true;
			}
		}
	}
	return false;
}

bool Utils::xlisionRects(float rect1X, float rect1Y, float rect1Width, float rect1Height, float rect2X, float rect2Y, float rect2Width, float rect2Height, float xPointDist) {
	//check points of second rect for intersection with first rect with given accuracy
	for (float x = rect2X; x < rect2X + rect2Width; x += xPointDist) {
		for (float y = rect2Y; y < rect2Y + rect2Height; y += xPointDist) {
			if (Utils::xlisionCoordsRect(rect1X, rect1Y, rect1Width, rect1Height, x, y) == true) {
				return true;
			}
		}
	}
	return false;
}

bool randSeedInit = true;
float Utils::RandomNumber(float Min, float MrectX) {
	if (randSeedInit == true) {
		srand(time(NULL));
		randSeedInit = false;
	}
	return ((float(rand()) / float(RAND_MAX)) * (MrectX - Min)) + Min;
}

bool Utils::compareFloats(float a, float b, float accuracy) {
	float movedA = a + 1000;
	float movedB = b + 1000;
	int roundA = (int)(movedA / accuracy);//a * 1 / accuracy
	int roundB = (int)(movedB / accuracy);
	return (roundA == roundB);
}

float Utils::calcDist1D(float a, float b) {
	float dist = 0;
	float absA = abs(a);
	float absB = abs(b);
	if (absA >= absB) {
		dist = absA - absB;
	}
	if (absB > absA) {
		dist = absB - absA;
	}
	return dist;
}

float Utils::calcDist2D(float x1, float x2, float y1, float y2) {
	return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}
