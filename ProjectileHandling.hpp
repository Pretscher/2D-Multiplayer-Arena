#pragma once
#include <vector>
#include "Projectile.hpp"
#include "Rect.hpp"
#include "Player.hpp"

class ProjectileHandling {
public:

	ProjectileHandling();
	
	void draw();
	void update(Rect** colidables, int colidableSize);
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

	int worldHeight, worldWidth;
	Player** players;
	int myPlayerI;
	int playerCount;
};