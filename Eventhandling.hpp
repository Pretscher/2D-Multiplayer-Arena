#pragma once
#include "Renderer.hpp"
#include "Player.hpp"
#include "Terrain.hpp"
#include "Projectile.hpp"
#include "PathfindingHandler.hpp"
#include "Utils.hpp"
#include "PortableClient.hpp"
#include "PortableServer.hpp"
#include "Menu.hpp"
#include "WorldHandling.hpp"
#include "UiHandling.hpp"
#include "ProjectileHandling.hpp"
#include "NetworkCommunication.hpp"
#include "Playerhandling.hpp"
#include "AbilityHandling.hpp"
#include "GlobalRecources.hpp"
#include "Graph.hpp"
#include <memory>

class Eventhandling {
public:
	Eventhandling();
	void eventloop();
	void drawingloop();


private:
	void sendData();
	void recvAndImplementData();

	unique_ptr<Menu> menu;
	bool menuActive = true;

	bool received = true;
	thread* networkThread;

	unique_ptr<PlayerHandling> playerHandling;
	unique_ptr<WorldHandling> worldHandling;
	unique_ptr<UiHandling> uiHandling;
	unique_ptr<Pathfinding> pathfinding;
	unique_ptr<ProjectileHandling> projectileHandling;

	unique_ptr<AbilityHandling> abilityHandling;
};