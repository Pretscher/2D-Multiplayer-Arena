#include "Eventhandling.hpp"
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


static std::unique_ptr<Menu> menu;
static bool menuActive = true;

static std::unique_ptr<PortableServer> server;
static std::unique_ptr<PortableClient> client;

static bool received = true;

static void initServer();
static void initClient();
static void sendData();
static void recvAndImplementData();
static std::thread* networkThread;

static std::unique_ptr<PlayerHandling> playerHandling;
static std::unique_ptr<WorldHandling> worldHandling;
static std::unique_ptr<UiHandling> uiHandling;
static std::unique_ptr<Pathfinding> pathfinding;
static std::unique_ptr<ProjectileHandling> projectileHandling;

static std::unique_ptr<AbilityHandling> abilityHandling;
void eventhandling::init() {
	int worldWidth = 1920, worldHeight = 1080, vsWidth = 2000, vsHeight = 2000;
	//be sure to not change the order, they depend on each other heavily
	playerHandling = std::unique_ptr<PlayerHandling>(new PlayerHandling());
	worldHandling = std::unique_ptr<WorldHandling>(new WorldHandling(worldWidth, worldHeight, vsWidth, vsHeight));
	uiHandling = std::unique_ptr<UiHandling>(new UiHandling());
	pathfinding = std::unique_ptr<Pathfinding>(new Pathfinding());
	projectileHandling = std::unique_ptr<ProjectileHandling>(new ProjectileHandling());
	menu = std::unique_ptr<Menu>(new Menu());
}

void eventhandling::eventloop() {
	//if host or client has not been selected, wait for it to happen
	if (menuActive == true) {
		menu->update();
		bool initIndex = false;
		int playerIndex;
		if (menu->hostServer() == true) {
			playerIndex = 0;
			initIndex = true;
			networkThread = new std::thread(&initServer);
			menuActive = false;//go to game
		}
		if (menu->connectAsClient() == true) {
			playerIndex = 1;
			initIndex = true;
			networkThread = new std::thread(&initClient);
			menuActive = false;//go to game
		}
		if (initIndex == true) {
			playerHandling->setPlayerIndex(playerIndex);//right now there are only two players so the client just has index 1
			pathfinding->setPlayerIndex(playerIndex);
			projectileHandling->setPlayerIndex(playerIndex);
			abilityHandling = std::unique_ptr<AbilityHandling>(new AbilityHandling(playerIndex));
		}
	}

	//if host or client has been selected
	if (menuActive == false) {
		worldHandling->update();
		//pass current hp informations to uiHandling so that it can draw a proper life bar
		uiHandling->updateLifeBar(playerHandling->getMyPlayer()->getHp(), playerHandling->getMyPlayer()->getMaxHp());
		//does pathfinding on click and player colision
		pathfinding->update();
		//moves all abilities and ticks through their states (example projectile->explosion->buring etc.)
		abilityHandling->update();

		//pass colidbales to projectile management every update so that projectiles can even be stopped by moving terrain
		auto colidables = worldHandling->getTerrain()->getCollidables();
		projectileHandling->update(colidables->data(), colidables->size());

		//pass game information back and forth through tcp sockets
		if ((server != nullptr && server->isConnected() == true) || (client != nullptr && client->isConnected() == true)) {
			if (received == true) {//handshaking: only if something was received send again. Prevents lag and unwanted behavior
				sendData();
				received = false;
			}
			recvAndImplementData();
		}
	}
}


void eventhandling::drawingloop() {
	if (menuActive == true) {
		menu->drawMenu();
	}
	else {
		worldHandling->draw();//draw first, lifebars and stuff should be drawn over it
		abilityHandling->drawAbilities();
		projectileHandling->draw();
		playerHandling->draw();
		uiHandling->draw();//draw last, should be over every item ingame
		abilityHandling->drawCDs();
	}
	//GlobalRecources::pFinding->getGraph()->debugDrawing();
}


//Neworking part-----------------------------------------------------------------------------------------------------------------


static void initServer() {
	server = std::unique_ptr<PortableServer>(new PortableServer());
	server->waitForClient();
	server->receiveMultithreaded();
}

static void initClient() {
	std::string s = "192.168.178.28";//TODO: typeable ip
	client = std::unique_ptr<PortableClient>(new PortableClient(s.c_str()));
	client->waitForServer();
	client->receiveMultithreaded();
}


static void sendData() {
	NetworkCommunication::initNewCommunication();

	abilityHandling->sendData();
	playerHandling->sendPlayerData();
	projectileHandling->sendProjectiles();


	if (playerHandling->getPlayerIndex() == 0) {
		NetworkCommunication::sendTokensToServer(std::move(*server.get()));
	}
	if (playerHandling->getPlayerIndex() == 1) {
		NetworkCommunication::sendTokensToClient(std::move(*client.get()));
	}
}

static void recvAndImplementData() {

	bool receivedSth = false;
	if (playerHandling->getPlayerIndex() == 0) {
		if (server->newMessage() == true) {
			receivedSth = true;
			NetworkCommunication::receiveTonkensFromServer(std::move(*server.get()));
		}
	}
	else {
		if (client->newMessage() == true) {
			receivedSth = true;
			NetworkCommunication::receiveTonkensFromClient(std::move(*client.get()));
		}
	}
	if (receivedSth == true) {
		abilityHandling->receiveData();
		playerHandling->receivePlayerData(std::move(*pathfinding.get()));
		projectileHandling->receiveProjectiles();

		received = true;
	}
}
