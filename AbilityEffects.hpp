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

	}
	
	void drawAll() {

	}

private:
	Player** players;
	int playerCount;
};
