#include "Player.hpp"
#include <iostream>
int* pathXpositions;
int* pathYpositions;
int pathLenght;

Player::Player(int i_col, int i_row, int i_width, int i_height, float i_vel) {
	this->col = i_col;
	this->row = i_row;
	this->velocity = i_vel;
	this->drawWidth = i_width;
	this->drawHeight = i_height;

	pathXpositions = nullptr;
	pathYpositions = nullptr;
	pathLenght = -1;
}

float cPathIndex;
void Player::givePath(int* i_pathX, int* i_pathY, int i_pathLenght) {
	//free memory in case of second path or more
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
		//go one step in path
		this->col = pathXpositions[roundPathIndex];
		this->row = pathYpositions[roundPathIndex];

		if (cPathIndex + velocity < pathLenght) {
			cPathIndex += velocity;
		}
		else {
			pathLenght = -1;
		}
	}
}