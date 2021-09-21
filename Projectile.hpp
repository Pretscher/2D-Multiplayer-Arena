#pragma once
#include "Rect.hpp"
#include "SFML/Graphics.hpp"
#include <iostream>
#include "Player.hpp"

class Projectile {
public:
	/**Calculates a line from player to goal (and beyond the goal in the same direction). 
	* @param playercoords, velocity of projectile, goalcoords, radius of projectile
	**/
	Projectile(int y, int x, float velocity, int goalY, int goalX, bool moveThroughGoal, int radius, shared_ptr<Player> shootingPlayer);
	/**Moves on the Line calculated in the constructor
	* @param maximum ys/xs where movement is allowed, array of Rect* and its lenght which cannot be passed through by the projectile
	**/
	void move(int maxY, int maxX, const unique_ptr<vector<Rect>>& colisionRects);
	void draw(sf::Color c) const;

	//Call this to skip some steps in projectile movement based on time, useful for networking
	void skipMovementTime(int i_skippedTime) {
		lastMoveTime -= i_skippedTime;//will skip some steps to catch up with time
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
	shared_ptr<Player> player;

	float movementFunc(float x) const {//used in constructor to calculate direction, f(x) = ax + b format
		return this->slope * x + this->yOffset;
	}
public:

	bool isDead() const {
		return this->dead;
	}

    inline float getY() const { return y; }
	inline void setY(float y) { this->y = y; }

	inline float getX() const { return x; }
	inline void setX(float x) { this->x = x; }

	inline int getGoalY() const { return goalY; }
	inline void setGoalY(int goalY) { this->goalY = goalY; }

	inline int getGoalX() const { return goalX; }
	inline void setGoalX(int goalX) { this->goalX = goalX; }


	inline bool getDead() const { return dead; }
	inline void setDead(bool dead) { this->dead = dead; }

	inline int getRadius() const { return radius; }
	inline void setRadius(int radius) { this->radius = radius; }

	inline shared_ptr<Player> getPlayer() const { return player; }
};
