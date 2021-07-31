#include "Player.hpp"
#include "Renderer.hpp"
#include <iostream>


Player::Player(int i_col, int i_row, int i_width, int i_height, float i_vel, int i_maxHp, int i_dmg) {
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
	currentTexture = textures[3];
	findingPath = false;
}


void Player::givePath(int* i_pathX, int* i_pathY, int i_pathLenght) {
	//free memory in case of reassigning path
	if (pathLenght != -1) {
		delete[] pathXpositions;
		delete[] pathYpositions;
	}
	pathXpositions = i_pathX;
	pathYpositions = i_pathY;
	cPathIndex = 0;
	pathLenght = i_pathLenght;
}

void Player::move() {
	if (pathLenght != -1) {//too lazy for booleans as you can see
		//has to be a float so that it can be modified by non-int velocities properly
		int roundPathIndex = int(cPathIndex);

		int nextCol = pathXpositions[roundPathIndex];
		int nextRow = pathYpositions[roundPathIndex];


		if (nextRow > row) {
			currentTexture = textures[3];
		}
		if (nextRow < row) {
			currentTexture = textures[2];
		}
		if (nextCol < col) {
			currentTexture = textures[0];
		}

		if (nextCol > col) {
			currentTexture = textures[1];
		}

		//go one step in path
		this->col = nextCol;
		this->row = nextRow;

		if (cPathIndex + velocity < pathLenght) {
			cPathIndex += velocity;
		}
		else {
			//go to state of not having a path
			pathLenght = -1;
			delete[] pathXpositions;
			delete[] pathYpositions;
		}
	}
}

void Player::deletePath() {
	if (pathLenght != -1) {
		pathLenght = -1;
		delete[] pathXpositions;
		delete[] pathYpositions;
	}
}

void Player::draw() {
	Renderer::drawRectWithTexture(row, col, width, height, currentTexture, false);
	
	int barWidth = width * 1.5;
	Renderer::drawRect(row - 40, col - (barWidth - width) / 2, barWidth, 30, sf::Color(20, 30, 20, 255), false);
	if (hp > 0) {
		float widthMult = (float)hp / maxHp;
		Renderer::drawRect(row - 40, col - (barWidth - width) / 2, barWidth * widthMult, 30, sf::Color(0, 100, 0, 255), false);
	}
}

void Player::initTextures() {
	textures = new sf::Texture[4];
	textures[0] = Renderer::loadTexture("Textures/mageFromAboveLeft.png");
	textures[1] = Renderer::loadTexture("Textures/mageFromAboveRight.png");
	textures[2] = Renderer::loadTexture("Textures/mageFromAboveTop.png");
	textures[3] = Renderer::loadTexture("Textures/mageFromAboveBottom.png");
}

void Player::setTexture(int index) {
	currentTexture = textures[index];
}