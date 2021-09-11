#pragma once

namespace Utils {
	bool collisionCoordsRect(float rectCol, float rectRow, float rectWidth, float rectHeight, float pointCol, float pointRow);
	/**
	* @param top left coords of and circle
	**/
	bool collisionCoordsCircle(float circleCol, float circleRow, float circleRadius, float pointCol, float pointRow);
	/**
	* @param top left coords of rect and circle, colPointDist is accuracy of detection
	**/
	bool collisionRectCircle(int rectCol, int rectRow, int rectWidth, int rectHeight, int circleCol, int circleRow, int circleRadius, int colPointDist);
	bool collisionRectCircleOnlyOutline(int rectCol, int rectRow, int rectWidth, int rectHeight, int circleCol, int circleRow, int circleRadius);

	bool collisionRects(float rect1Col, float rect1Row, float rect1Width, float rect1Height, float rect2Col, float rect2Row, float rect2Width, float rect2Height, float colPointDist);
	float RandomNumber(float Min, float Max);
	bool compareFloats(float a, float b, float accuracy);
	float calcDist1D(float a, float b);
	float calcDist2D(float col1, float col2, float row1, float row2);

};