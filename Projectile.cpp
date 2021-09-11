#include "Projectile.hpp"
#include "Renderer.hpp"
#include <iostream>
#include "Utils.hpp"
#include "Player.hpp"
#include <chrono>

Projectile::Projectile(int i_y, int i_x, float velocity, int i_goalY, int i_goalX, 
		bool i_moveThroughGoal, int radius, Player* shootingPlayer) {
	
	this->player = shootingPlayer;
	this->y = i_y - radius;//not same as player position, changes with direction because player rotates
	this->x = i_x - radius;
	this->vel = velocity;
	this->radius = radius;
	this->goalY = i_goalY;
	this->goalX = i_goalX;
	this->moveThroughGoal = i_moveThroughGoal;

	//we want to push those projectiles through network with the same goaly and goalx so dont change them here. 
	i_goalY -= radius;
	i_goalX -= radius;

	this->dead = false;
	//calculate function for line
	this->slope = 0;
	this->yOffset = 0;
	if (i_goalX > x) {
		this->slope = (float)(i_goalY - this->y) / (float)(i_goalX - this->x);
	}
	else {
		this->slope = (float)(this->y - i_goalY) / (float)(this->x - i_goalX);
	}
	//f(x1) = slope * x1 + yOffset <=> f(x1) - slope * x1 = yOffset (where f(x1) is obviously y1)
	this->yOffset = (float)this->y - (this->slope * (float)this->x);
	if (i_goalX > this->x) this->up = true;
	else this->up = false;

	auto timePoint = std::chrono::system_clock::now().time_since_epoch();
	lastMoveTime = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint).count();
}

void Projectile::move(int maxY, int maxX, Rect** xlisionRects, int rectCount) {

	auto timePoint = std::chrono::system_clock::now().time_since_epoch();
	long long now = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint).count();
	long long diff = now - this->lastMoveTime;
	float dueSteps = diff / this->vel;

	if (dueSteps >= 1.0f) {
		lastMoveTime = now;
	}
	//move gradually between path steps
	for (int i = 0; i < (int)dueSteps; i++) {
			float gotoDiff = this->vel * 2;

			float nextX = 0;
			float nextY = 0;

			if (this->up == true) {
				nextX = this->x + this->vel;
			}
			else {
				nextX = this->x - this->vel;
			}
			nextY = movementFunc(x);

			//save if y is negative
			bool negY = false;
			if (nextY < y) negY = true;

			float yDiff = abs(nextY - y);
			float xDiff = abs(nextX - x);

			float sum = gotoDiff / (xDiff + yDiff);
			yDiff *= sum;
			xDiff *= sum;
			if (negY == false) {
				nextY = this->y + yDiff;
			}
			else {
				nextY = this->y - yDiff;
			}
			if (up == true) {
				nextX = this->x + xDiff;
			}
			else {
				nextX = this->x - xDiff;
			}

			//check if out of window bounds (TODO: COLLISISION WITH PLAYERS)
			if (nextX > maxX || nextX < 0 || nextY > maxX || nextY < 0) {
				dead = true;
				return;
			}

			for (unsigned int i = 0; i < rectCount; i++) {
				Rect* cRect = xlisionRects[i];
				if (Utils::xlisionRectCircleOnlyOutline(cRect->getX(), cRect->getY(), cRect->getWidth(), cRect->getHeight(), nextX, nextY, this->radius) == true) {
					dead = true;
					return;
				}
			}
			//no xlisiom => move on function
			this->y = nextY;
			this->x = nextX;

			if (moveThroughGoal == false) {
				if (Utils::calcDist2D(goalX - radius, x, goalY - radius, y) < 50) {
					dead = true;
					return;
				}
			}
		}
}

void Projectile::draw(sf::Color c) {
	Renderer::drawCircle((int)this->y, (int)this->x, this->radius, c, true, 0, false);
}
