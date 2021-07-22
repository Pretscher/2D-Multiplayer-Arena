#pragma once
#include "Rect.hpp"

class Projectile {
public:
	/**Calculates a line from player to goal (and beyond the goal in the same direction). 
	* @param playercoords, velocity of projectile, goalcoords, radius of projectile
	**/
	Projectile(int playerRow, int playerCol, float velocity, int goalRow, int goalCol, int radius);
	/**Moves on the Line calculated in the constructor
	* @param maximum rows/cols where movement is allowed, array of Rect* and its lenght which cannot be passed through by the projectile
	**/
	void move(int maxRow, int maxCol, Rect** collisionRects, int rectCount);
	void draw();


	bool isDead() {
		return this->dead;
	}

private:
	float row;
	float col;
	float vel;
	float slope;
	float yOffset;
	bool up;
	bool dead;
	int radius;
	float movementFunc(float x) {//used in constructor to calculate direction, f(x) = ax + b format
		return this->slope * x + this->yOffset;
	}
};
