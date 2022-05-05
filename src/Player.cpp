#include "Player.hpp"
#include "Renderer.hpp"
#include "iostream" 
using namespace std; 
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
	pathLenght = -1;

	this->initTextures();
	cTextureI = 3;//lookin down
	findingPath = false;
}


void Player::givePath(vector<int> i_pathX, vector<int> i_pathY, int i_pathLenght) {
	GlobalRecources::pfMtx->lock();
	//free memory in case of reassigning path
	pathXpositions = std::move(i_pathX);
	pathYpositions = std::move(i_pathY);
	cPathIndex = 0;
	pathLenght = i_pathLenght;
	pathsFound++;
	if (pathLenght > 0) {
		hasNewPath = true;
	}
	GlobalRecources::pfMtx->unlock();

	auto timePoint = chrono::system_clock::now().time_since_epoch();
	lastMoveTime = chrono::duration_cast<chrono::milliseconds>(timePoint).count();

}


void Player::move() {
	auto timePoint = chrono::system_clock::now().time_since_epoch();
	long long now = chrono::duration_cast<chrono::milliseconds>(timePoint).count();
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

				if (cPathIndex == pathLenght - 1) {//arrived at end of path, path data can now be deleted
					y = pathYpositions[pathLenght - 1];
					x = pathXpositions[pathLenght - 1];
					//go to state of not having a path
					pathLenght = -1;

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
		auto timePoint = chrono::system_clock::now().time_since_epoch();
		lastMoveTime = chrono::duration_cast<chrono::milliseconds>(timePoint).count();
	}
}

void Player::deletePath() {
	GlobalRecources::pfMtx->lock();
	if (pathLenght != -1) {
		pathLenght = -1;
		interruptedPath = true;
	}
	findingPath = false;
	hasNewPath = false;
	GlobalRecources::pfMtx->unlock();
}

void Player::draw() {
	if (inVladW == false) {
		Renderer::drawRectWithTexture(x, y, width, height, textures[cTextureI], false);
	}
	int barWidth = width * 1.5;
	Renderer::drawRect(x - (barWidth - width) / 2, y - 40, barWidth, 30, sf::Color(20, 30, 20, 255), false);
	if (hp > 0) {
		float widthMult = (float)hp / maxHp;
		Renderer::drawRect(x - (barWidth - width) / 2, y - 40, barWidth * widthMult, 30, sf::Color(0, 100, 0, 255), false);
	}
}

void Player::initTextures() {
	textures = vector<sf::Texture>(4);
	textures[0] = Renderer::loadTexture("myRecources/Textures/mageFromAboveLeft.png", false);
	textures[1] = Renderer::loadTexture("myRecources/Textures/mageFromAboveRight.png", false);
	textures[2] = Renderer::loadTexture("myRecources/Textures/mageFromAboveTop.png", false);
	textures[3] = Renderer::loadTexture("myRecources/Textures/mageFromAboveBottom.png", false);
}

void Player::setTexture(int index) {
	cTextureI = index;
}

void Player::setFindingPath(bool i_findingPath) {
	GlobalRecources::pfMtx->lock();
	findingPath = i_findingPath;
	GlobalRecources::pfMtx->unlock();
}

bool Player::isFindingPath() const {
	bool temp;
	GlobalRecources::pfMtx->lock();
	temp = findingPath;
	GlobalRecources::pfMtx->unlock();
	return temp;
}

void Player::skipPathToIndex(int index) {
	GlobalRecources::pfMtx->lock();
	if (index >= pathLenght - 1) {
		this->x = pathXpositions[pathLenght - 1];
		this->y = pathYpositions[pathLenght - 1];
		pathLenght = -1;
		hasNewPath = false;
	}
	else {
		this->x = pathXpositions[index];
		this->y = pathYpositions[index];
		cPathIndex = index;
	}
	GlobalRecources::pfMtx->unlock();
}