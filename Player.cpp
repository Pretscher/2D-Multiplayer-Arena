#include "Player.hpp"
#include "Renderer.hpp"
#include <iostream>
#include <chrono>
#include <math.h>
#include "GlobalRecources.hpp"

Player::Player(int i_col, int i_row, int i_width, int i_height, float i_vel, float i_maxHp, int i_dmg) {
	this->col = i_col;
	this->row = i_row;
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
		int nextCol = pathXpositions [cPathIndex];
		int nextRow = pathYpositions [cPathIndex];

		float colDiff = (sqrt(((float) col - nextCol) * ((float) col - nextCol)) * dueSteps);
		float rowDiff = (sqrt(((float) row - nextRow) * ((float) row - nextRow)) * dueSteps);
		if (col < nextCol) {
			col = (float) col + colDiff;
		}
		else {
			col = (float) col - colDiff;
		}
		if (row < nextRow) {
			row = (float) row + rowDiff;
		}
		else {
			row = (float) row - rowDiff;
		}
	}

	if (dueSteps >= 1) {
		auto timePoint = std::chrono::system_clock::now().time_since_epoch();
		lastMoveTime = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint).count();
		if (pathLenght != -1) {//too lazy for booleans as you can see
			//has to be a float so that it can be modified by non-int velocities properly

			int nextCol;
			int nextRow;
			for (int i = 0; i <= dueSteps; i++) {
				if (pathLenght == -1) return;
				nextCol = pathXpositions [cPathIndex];
				nextRow = pathYpositions [cPathIndex];

				if (cPathIndex < pathLenght - 2) {
					cPathIndex ++;
				}
				else {
					row = pathYpositions [pathLenght - 1];
					col = pathXpositions [pathLenght - 1];
					//go to state of not having a path
					pathLenght = -1;
					delete [] pathXpositions;
					delete [] pathYpositions;

					hasNewPath = false;
					return;
				}
			}

			if (nextRow > row) {
				cTextureI = 3;
			}
			if (nextRow < row) {
				cTextureI = 2;
			}
			if (nextCol < col) {
				cTextureI = 0;
			}

			if (nextCol > col) {
				cTextureI = 1;
			}

			//go one step in path
			this->col = nextCol;
			this->row = nextRow;
		}
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
	Renderer::drawRectWithTexture(row, col, width, height, textures[cTextureI], false);
	
	int barWidth = width * 1.5;
	Renderer::drawRect(row - 40, col - (barWidth - width) / 2, barWidth, 30, sf::Color(20, 30, 20, 255), false);
	if (hp > 0) {
		float widthMult = (float)hp / maxHp;
		Renderer::drawRect(row - 40, col - (barWidth - width) / 2, barWidth * widthMult, 30, sf::Color(0, 100, 0, 255), false);
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