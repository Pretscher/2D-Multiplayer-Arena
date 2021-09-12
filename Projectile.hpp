#pragma once
#include "Rect.hpp"
#include "SFML/Graphics.hpp"

class Player;
class Projectile {
public:
	/**Calculates a line from player to goal (and beyond the goal in the same direction). 
	* @param playercoords, velocity of projectile, goalcoords, radius of projectile
	**/
	Projectile(int y, int x, float velocity, int goalY, int goalX, bool moveThroughGoal, int radius, Player* shootingPlayer);
	/**Moves on the Line calculated in the constructor
	* @param maximum ys/xs where movement is allowed, array of Rect* and its lenght which cannot be passed through by the projectile
	**/
	void move(int maxY, int maxX, Rect** colisionRects, int rectCount);
	void draw(sf::Color c);

	//Call this to skip some steps in projectile movement based on time, useful for networking
	void skipMovementTime(int i_skippedTime) {
		lastMoveTime -= i_skippedTime;//will skip some steps to catch up with time
	}

	bool isDead() {
		return this->dead;
	}

private:
	bool moveThroughGoal;
	long long lastMoveTime;
	int goalY;
	int goalX;
	float y;
	float x;
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
    inline float getY() { return y; }
	inline void setY(float y) { this->y = y; }

	inline float getX() { return x; }
	inline void setX(float x) { this->x = x; }

	inline int getGoalY() { return goalY; }
	inline void setGoalY(int goalY) { this->goalY = goalY; }

	inline int getGoalX() { return goalX; }
	inline void setGoalX(int goalX) { this->goalX = goalX; }


	inline bool getDead() { return dead; }
	inline void setDead(bool dead) { this->dead = dead; }

	inline int getRadius() { return radius; }
	inline void setRadius(int radius) { this->radius = radius; }

	inline Player* getPlayer() { return player; }
};
