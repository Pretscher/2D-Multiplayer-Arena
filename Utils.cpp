#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include "Utils.hpp"

bool Utils::collisionCoordsRect(float rectCol, float rectRow, float rectWidth, float rectHeight, float pointCol, float pointRow) {
	if (rectCol < pointCol && rectCol + rectWidth > pointCol) {
		if (rectRow < pointRow && rectRow + rectHeight > pointRow) {
			return true;
		}
	}
	return false;
}
/**
* @param top left coords of circle and point coords
**/
bool Utils::collisionCoordsCircle(float circleCol, float circleRow, float circleRadius, float pointCol, float pointRow) {
	float dist = sqrt((pointCol - (circleCol + circleRadius)) * (pointCol - (circleCol + circleRadius)) + (pointRow - (circleRow + circleRadius)) * (pointRow - (circleRow + circleRadius)));
	if (dist <= circleRadius) return true;
	return false;
}

/**
* @param top left coords of rect and circle, colPointDist is accuracy of detection
**/
bool Utils::collisionRectCircle(int rectCol, int rectRow, int rectWidth, int rectHeight, int circleCol, int circleRow, int circleRadius, int colPointDist) {
	//check points of second rect for intersection with circle with given accuracy
	for (float col = rectCol; col <= rectCol + rectWidth; col += colPointDist) {
		for (float row = rectRow; row <= rectRow + rectHeight; row += colPointDist) {
			//Renderer::drrectWidthRect(row, col, 2, 2, sf::Color(255, 255, 0, 255), false);
			if (Utils::collisionCoordsCircle(circleCol, circleRow, circleRadius, col, row) == true) {
				return true;
			}
		}
	}
	return false;
}

bool Utils::collisionRectCircleOnlyOutline(int rectCol, int rectRow, int rectWidth, int rectHeight, int circleCol, int circleRow, int circleRadius) {
	//check points of second rect for intersection with circle with given accuracy
	for (float row = rectRow; row <= rectRow + rectHeight; row += rectHeight) {
		for (float col = rectCol; col <= rectCol + rectWidth; col += (circleRadius * 2 - 1)) {
			//Renderer::drrectWidthRect(row, col, 2, 2, sf::Color(255, 255, 0, 255));
			if (Utils::collisionCoordsCircle(circleCol, circleRow, circleRadius, col, row) == true) {
				return true;
			}
		}
	}
	for (float col = rectCol; col <= rectCol + rectWidth; col += rectWidth) {
		for (float row = rectRow; row <= rectRow + rectHeight; row += (circleRadius * 2 - 1)) {
			//Renderer::drrectWidthRect(row, col, 2, 2, sf::Color(255, 255, 0, 255));
			if (Utils::collisionCoordsCircle(circleCol, circleRow, circleRadius, col, row) == true) {
				return true;
			}
		}
	}
	return false;
}

bool Utils::collisionRects(float rect1Col, float rect1Row, float rect1Width, float rect1Height, float rect2Col, float rect2Row, float rect2Width, float rect2Height, float colPointDist) {
	//check points of second rect for intersection with first rect with given accuracy
	for (float col = rect2Col; col < rect2Col + rect2Width; col += colPointDist) {
		for (float row = rect2Row; row < rect2Row + rect2Height; row += colPointDist) {
			if (Utils::collisionCoordsRect(rect1Col, rect1Row, rect1Width, rect1Height, col, row) == true) {
				return true;
			}
		}
	}
	return false;
}

bool randSeedInit = true;
float Utils::RandomNumber(float Min, float MrectCol) {
	if (randSeedInit == true) {
		srand(time(NULL));
		randSeedInit = false;
	}
	return ((float(rand()) / float(RAND_MAX)) * (MrectCol - Min)) + Min;
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

float Utils::calcDist2D(float col1, float col2, float row1, float row2) {
	return sqrt((col1 - col2) * (col1 - col2) + (row1 - row2) * (row1 - row2));
}
