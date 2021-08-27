#include "Projectile.hpp"
#include "Renderer.hpp"
#include <iostream>
#include "Utils.hpp"
#include "Player.hpp"
#include <chrono>

Projectile::Projectile(int i_row, int i_col, float velocity, int i_goalRow, int i_goalCol, int radius, Player* shootingPlayer) {
	this->player = shootingPlayer;
	this->row = i_row - radius;//not same as player position, changes with direction because player rotates
	this->col = i_col - radius;
	this->vel = velocity;
	this->radius = radius;
	this->goalRow = i_goalRow;
	this->goalCol = i_goalCol;

	//we want to push those projectiles through network with the same goalrow and goalcol so dont change them here. 
	i_goalRow -= radius;
	i_goalCol -= radius;


	this->dead = false;
	//calculate function for line
	this->slope = 0;
	this->yOffset = 0;
	if (i_goalCol > col) {
		this->slope = (float)(i_goalRow - this->row) / (float)(i_goalCol - this->col);
	}
	else {
		this->slope = (float)(this->row - i_goalRow) / (float)(this->col - i_goalCol);
	}
	//f(x1) = slope * x1 + yOffset <=> f(x1) - slope * x1 = yOffset (where f(x1) is obviously y1)
	this->yOffset = (float)this->row - (this->slope * (float)this->col);
	if (i_goalCol > this->col) this->up = true;
	else this->up = false;

	auto timePoint = std::chrono::system_clock::now().time_since_epoch();
	lastMoveTime = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint).count();
}

void Projectile::move(int maxRow, int maxCol, Rect** collisionRects, int rectCount) {

	auto timePoint = std::chrono::system_clock::now().time_since_epoch();
	long now = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint).count();
	long diff = now - this->lastMoveTime;
	float dueSteps = diff / this->vel;

	if (dueSteps >= 1.0f) {
		lastMoveTime = now;
	}
	//move gradually between path steps
	for (int i = 0; i < (int)dueSteps; i++) {
			float gotoDiff = this->vel * 2;

			float nextCol = 0;
			float nextRow = 0;

			if (this->up == true) {
				nextCol = this->col + this->vel;
			}
			else {
				nextCol = this->col - this->vel;
			}
			nextRow = movementFunc(col);

			//save if row is negative
			bool negRow = false;
			if (nextRow < row) negRow = true;

			float rowDiff = abs(nextRow - row);
			float colDiff = abs(nextCol - col);

			float sum = gotoDiff / (colDiff + rowDiff);
			rowDiff *= sum;
			colDiff *= sum;
			if (negRow == false) {
				nextRow = this->row + rowDiff;
			}
			else {
				nextRow = this->row - rowDiff;
			}
			if (up == true) {
				nextCol = this->col + colDiff;
			}
			else {
				nextCol = this->col - colDiff;
			}

			//check if out of window bounds (TODO: COLLISISION WITH PLAYERS)
			if (nextCol > maxCol || nextCol < 0 || nextRow > maxCol || nextRow < 0) {
				dead = true;
				return;
			}

			for (unsigned int i = 0; i < rectCount; i++) {
				Rect* cRect = collisionRects [i];
				if (Utils::collisionRectCircleOnlyOutline(cRect->getCol(), cRect->getRow(), cRect->getWidth(), cRect->getHeight(), nextCol, nextRow, this->radius) == true) {
					dead = true;
					return;
				}
			}

			//no collisiom => move on function
			this->row = nextRow;
			this->col = nextCol;
		}
}

void Projectile::draw(sf::Color c) {
	Renderer::drawCircle((int)this->row, (int)this->col, this->radius, c, true, 0, false);
}
