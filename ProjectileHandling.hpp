#pragma once
#include <vector>
#include "Projectile.hpp"
#include "Rect.hpp"
#include "Player.hpp"

class ProjectileHandling {
public:

	ProjectileHandling(int worldYs, int worldXs, Player** players, int playerCount);
	
	void draw();
	void update(Rect** xlidables, int xlidableSize);
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

	int worldYs, worldXs;
	Player** players;
	int myPlayerI;
	int playerCount;
};