#pragma once
#include <vector>
#include "Projectile.hpp"
#include "Rect.hpp"
#include "Player.hpp"

class ProjectileHandling {
public:

	ProjectileHandling(int worldRows, int worldCols, Player** players, int playerCount);
	
	void draw();
	void update(Rect** collidables, int collidableSize);
	void sendProjectiles();
	void receiveProjectiles();

	void setPlayerIndex(int playerIndex) {
		myPlayerI = playerIndex;
	}

private:
	float projectileVel;
	int projectileRadius;
	bool samePress = false;
	std::vector<Projectile*>* newProjectiles;
	std::vector<Projectile*>* projectiles;//stores all projectiles for creation, drawing, moving and damage calculation. 

	int worldRows, worldCols;
	Player** players;
	int myPlayerI;
	int playerCount;
};