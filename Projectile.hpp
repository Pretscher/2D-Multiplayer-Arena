#pragma once
#include "Rect.hpp"
#include "Utils.hpp"


class Projectile {
public:
	Projectile(int playerRow, int playerCol, float velocity, int goalRow, int goalCol, int radius) {
		playerRow -= radius;
		playerCol -= radius;
		this->row = playerRow;
		this->col = playerCol;
		this->vel = velocity;
		this->radius = radius;

		goalRow -= radius;
		goalCol -= radius;


		this->dead = false;
		//calculate function for line
		this->slope = 0;
		this->yOffset = 0;
		if (goalCol > playerCol) {
			this->slope = (float)(goalRow - playerRow) / (float)(goalCol - playerCol);
		}
		else {
			this->slope = (float)(playerRow - goalRow) / (float)(playerCol - goalCol);
		}
		//f(x1) = slope * x1 + yOffset <=> f(x1) - slope * x1 = yOffset (where f(x1) is obviously y1)
		this->yOffset = (float)playerRow - (this->slope * (float)playerCol);
		if (goalCol > playerCol) this->up = true;
		else this->up = false;
	}

	void move(int maxRow, int maxCol, Rect** collisionRects, int rectCount) {
		float gotoDiff = 10.0f;

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
			Rect* cRect = collisionRects[i];
			if (Utils::collisionRectCircleOnlyOutline(cRect->getCol(), cRect->getRow(), cRect->getWidth(), cRect->getHeight(), nextCol, nextRow, this->radius) == true) {
				dead = true;
				return;
			}
		}

		//no collisiom => move on function
		this->row = nextRow;
		this->col = nextCol;
	}

	void draw() {
		Renderer::drawCircle((int)this->row, (int)this->col, this->radius, sf::Color(255, 0, 255, 255), true);
	}

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
	float movementFunc(float x) {
		return this->slope * x + this->yOffset;
	}
};
