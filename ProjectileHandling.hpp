#pragma once
#include "iostream" 
using namespace std;
#include <vector>
#include "Projectile.hpp"
#include "Rect.hpp"
#include "Player.hpp"
#include <memory>
class ProjectileHandling {
public:

	ProjectileHandling();
	
	void draw();
	void update(const shared_ptr<vector<Rect>> colidables);
	void sendProjectiles(int socketIndex);
	void receiveProjectiles(int index);

	void setPlayerIndex(int playerIndex) {
		myPlayerI = playerIndex;
	}

private:
	float projectileVel;
	int projectileRadius;
	bool samePress = false;
	vector<shared_ptr<Projectile>> newProjectiles;//needs to be shared ptr caus it gets pushed back to newProjectiles and projectiles.
	vector<shared_ptr<Projectile>> projectiles;//stores all projectiles for creation, drawing, moving and damage calculation. 

	int worldHeight, worldWidth;
	shared_ptr<vector<shared_ptr<Player>>> players;
	int myPlayerI;
	int playerCount;
};