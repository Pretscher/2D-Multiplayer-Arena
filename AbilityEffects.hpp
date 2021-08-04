#pragma once
#include "Player.hpp"
#include "Renderer.hpp"

class abilityManagement {
public:
	abilityManagement(Player** players, int playerCount) {
		this->players = players;
		this->playerCount = playerCount;
	}

	void update() {

		checkForCreation();
	}

	void checkForCreation() {

	}
	
	void drawAll() {

	}

private:
	Player** players;
	int playerCount;
};

class ProjectileAbility {
public:
	ProjectileAbility() {

	}
private:
	int row, col;
};
