#pragma once
#include "Rect.hpp"
#include "SFML/Graphics.hpp"

class Player;
class Projectile {
public:
	/**Calculates a line from player to goal (and beyond the goal in the same direction). 
	* @param playercoords, velocity of projectile, goalcoords, radius of projectile
	**/
	Projectile(int row, int col, float velocity, int goalRow, int goalCol, int radius, Player* shootingPlayer);
	/**Moves on the Line calculated in the constructor
	* @param maximum rows/cols where movement is allowed, array of Rect* and its lenght which cannot be passed through by the projectile
	**/
	void move(int maxRow, int maxCol, Rect** collisionRects, int rectCount);
	void draw(sf::Color c);


	bool isDead() {
		return this->dead;
	}

private:
	long lastMoveTime;
	int goalRow;
	int goalCol;
	float row;
	float col;
	bool dead;
	int radius;

	float vel;
	float slope;
	float yOffset;
	bool up;
	Player* player;

	float movementFunc(float x) {//used in constructor to calculate direction, f(x) = ax + b format
		return this->slope * x + this->yOffset;
	}
public:
    inline float getRow() { return row; }
	inline void setRow(float row) { this->row = row; }

	inline float getCol() { return col; }
	inline void setCol(float col) { this->col = col; }

	inline int getGoalRow() { return goalRow; }
	inline void setGoalRow(int goalRow) { this->goalRow = goalRow; }

	inline int getGoalCol() { return goalCol; }
	inline void setGoalCol(int goalCol) { this->goalCol = goalCol; }


	inline bool getDead() { return dead; }
	inline void setDead(bool dead) { this->dead = dead; }

	inline int getRadius() { return radius; }
	inline void setRadius(int radius) { this->radius = radius; }

	inline Player* getPlayer() { return player; }
};
