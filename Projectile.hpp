#pragma once




class Projectile {
public:
	Projectile(int playerRow, int playerCol, float velocity, int goalRow, int goalCol) {
		this->row = playerRow;
		this->col = playerCol;
		this->vel = velocity;

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

	void move(int maxRow, int maxCol) {
		float nextCol = 0;
		float nextRow = 0;
		if (this->up == true) {
			nextCol = this->col + this->vel;
		}
		else {
			nextCol = this->col - this->vel;
		}
		nextRow = movementFunc(col);
		//check if out of window bounds (TODO: COLLISISION WITH PLAYERS)
		if (nextCol > maxCol || nextCol < 0 || nextRow > maxCol || nextRow < 0) {
			dead = true;
			return;
		}
		//no collisiom => move on function
		this->row = nextRow;
		this->col = nextCol;
	}

	void draw() {
		Renderer::drawCircle((int)this->row, (int)this->col, 50, sf::Color(255, 0, 255, 255), true);
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
	float movementFunc(float x) {
		return this->slope * x + this->yOffset;
	}
};
