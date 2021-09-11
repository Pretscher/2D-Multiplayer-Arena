#include "Player.hpp"
#include "Renderer.hpp"
#include <iostream>
#include <chrono>
#include <math.h>
#include "GlobalRecources.hpp"

Player::Player(int i_x, int i_y, int i_width, int i_height, float i_vel, float i_maxHp, int i_dmg) {
	this->x = i_x;
	this->y = i_y;
	this->velocity = i_vel;
	this->width = i_width;
	this->height = i_height;
	this->hp = i_maxHp;
	this->maxHp = i_maxHp;
	this->dmg = i_dmg;

	pathXpositions = nullptr;
	pathYpositions = nullptr;
	pathLenght = -1;

	this->initTextures();
	cTextureI = 3;//lookin down
	findingPath = false;
}


void Player::givePath(int* i_pathX, int* i_pathY, int i_pathLenght) {
	GlobalRecources::pfMtx->lock();
	//free memory in case of reassigning path
	if (pathLenght != -1) {
		delete[] pathXpositions;
		delete[] pathYpositions;
	}
	pathXpositions = i_pathX;
	pathYpositions = i_pathY;
	cPathIndex = 0;
	pathLenght = i_pathLenght;
	pathsFound++;
	if (pathLenght > 0) {
		hasNewPath = true;
	}
	GlobalRecources::pfMtx->unlock();

	auto timePoint = std::chrono::system_clock::now().time_since_epoch();
	lastMoveTime = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint).count();

}


void Player::move() {
	auto timePoint = std::chrono::system_clock::now().time_since_epoch();
	long long now = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint).count();
	long long diff = now - lastMoveTime;
	float dueSteps = diff / velocity;
	
	//move gradually between path steps
	if (dueSteps < 1) {
		int nextX = pathXpositions[cPathIndex];
		int nextY = pathYpositions[cPathIndex];

		float xDiff = (sqrt(((float) x - nextX) * ((float) x - nextX)) * dueSteps);
		float yDiff = (sqrt(((float) y - nextY) * ((float) y - nextY)) * dueSteps);
		if (x < nextX) {
			x = (float) x + xDiff;
		}
		else {
			x = (float) x - xDiff;
		}
		if (y < nextY) {
			y = (float) y + yDiff;
		}
		else {
			y = (float) y - yDiff;
		}
	}
	

	if (dueSteps >= 1) {
		if (pathLenght != -1) {//too lazy for booleans as you can see
			//has to be a float so that it can be modified by non-int velocities properly

			int nextX;
			int nextY;
			for (int i = 0; i <= dueSteps; i++) {
				if (pathLenght == -1) return;
				nextX = pathXpositions[cPathIndex];
				nextY = pathYpositions[cPathIndex];

				if (cPathIndex == pathLenght - 1) {
					y = pathYpositions[pathLenght - 1];
					x = pathXpositions[pathLenght - 1];
					//go to state of not having a path
					pathLenght = -1;
					delete[] pathXpositions;
					delete[] pathYpositions;

					hasNewPath = false;
				}
				if (cPathIndex < pathLenght - 1) {
					cPathIndex ++;
				}
			}

			//change texture based on movement direction
			if (nextY > y) {
				cTextureI = 3;
			}
			if (nextY < y) {
				cTextureI = 2;
			}
			if (nextX < x) {
				cTextureI = 0;
			}
			if (nextX > x) {
				cTextureI = 1;
			}

			//go one step in path
			this->x = nextX;
			this->y = nextY;
		}
		auto timePoint = std::chrono::system_clock::now().time_since_epoch();
		lastMoveTime = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint).count();
	}
}

void Player::deletePath() {
	GlobalRecources::pfMtx->lock();
	if (pathLenght != -1) {
		pathLenght = -1;
		delete[] pathXpositions;
		delete[] pathYpositions;
	}
	findingPath = false;
	hasNewPath = false;
	GlobalRecources::pfMtx->unlock();
}

void Player::draw() {
	if (inVladW == false) {
		Renderer::drawRectWithTexture(y, x, width, height, textures[cTextureI], false);
	}
	int barWidth = width * 1.5;
	Renderer::drawRect(y - 40, x - (barWidth - width) / 2, barWidth, 30, sf::Color(20, 30, 20, 255), false);
	if (hp > 0) {
		float widthMult = (float)hp / maxHp;
		Renderer::drawRect(y - 40, x - (barWidth - width) / 2, barWidth * widthMult, 30, sf::Color(0, 100, 0, 255), false);
	}
}

void Player::initTextures() {
	textures = new sf::Texture[4];
	textures[0] = Renderer::loadTexture("Textures/mageFromAboveLeft.png", false);
	textures[1] = Renderer::loadTexture("Textures/mageFromAboveRight.png", false);
	textures[2] = Renderer::loadTexture("Textures/mageFromAboveTop.png", false);
	textures[3] = Renderer::loadTexture("Textures/mageFromAboveBottom.png", false);
}

void Player::setTexture(int index) {
	cTextureI = index;
}

void Player::setFindingPath(bool i_findingPath) {
	GlobalRecources::pfMtx->lock();
	findingPath = i_findingPath;
	GlobalRecources::pfMtx->unlock();
}

bool Player::isFindingPath() {
	bool temp;
	GlobalRecources::pfMtx->lock();
	temp = findingPath;
	GlobalRecources::pfMtx->unlock();
	return temp;
}