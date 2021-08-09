#include "Eventhandling.hpp"
#include "Renderer.hpp"
#include "Player.hpp"
#include "Terrain.hpp"
#include "Projectile.hpp"
#include "PathfindingHandler.hpp"
#include "Utils.hpp"
#include "GameServer.hpp"
#include "GameClient.hpp"
#include "Menu.hpp"
#include "WorldHandling.hpp"
#include "UiHandling.hpp"
#include "ProjectileHandling.hpp"
#include "NetworkCommunication.hpp"
#include "Playerhandling.hpp"

Pathfinding* pathfinding;

Menu* menu;
bool menuActive = true;

GameServer* server;
GameClient* client;

UiHandling* uiHandling;
WorldHandling* worldHandling;
ProjectileHandling* projectileHandling;
PlayerHandling* playerHandling;

int NetworkCommunication::tokenCount;
std::string* NetworkCommunication::rawData;
std::vector<int>* NetworkCommunication::parseToIntsData;
int NetworkCommunication::tokenIndex;
bool received = true;
std::mutex* nMutex = new std::mutex();

void initServer();
void initClient();
void sendData();
void recvAndImplementData();
std::thread* networkThread;



void eventhandling::init() {
	//be sure to not change the order, they depend on each other heavily
	playerHandling = new PlayerHandling();
	worldHandling = new WorldHandling();
	uiHandling = new UiHandling(worldHandling->frameRows, worldHandling->frameCols);
	pathfinding = new Pathfinding(worldHandling->worldRows, worldHandling->worldCols, worldHandling->terrain,
		playerHandling->getPlayers(), playerHandling->getPlayerCount());
	projectileHandling = new ProjectileHandling(worldHandling->worldRows, worldHandling->worldCols,
		playerHandling->getPlayers(), playerHandling->getPlayerCount());
	NetworkCommunication::init();

	menu = new Menu();
}

void eventhandling::eventloop() {
	//if host or client has not been selected, wait for it to happen
	if (menuActive == true) {
		menu->update();
		if (menu->hostServer() == true) {
			playerHandling->setPlayerIndex(0);//server has player index 0
			pathfinding->setPlayerIndex(0);
			projectileHandling->setPlayerIndex(0);
			networkThread = new std::thread(&initServer);
			menuActive = false;//go to game
		}
		if (menu->connectAsClient() == true) {
			playerHandling->setPlayerIndex(1);//right now there are only two players so the client just has index 1
			pathfinding->setPlayerIndex(1);
			projectileHandling->setPlayerIndex(1);
			networkThread = new std::thread(&initClient);
			menuActive = false;//go to game
		}
	}

	//if host or client has been selected
	if (menuActive == false) {
		worldHandling->update();
		//pass current hp informations to uiHandling so that it can draw a proper life bar
		uiHandling->updateLifeBar(playerHandling->getMyPlayer()->getHp(), playerHandling->getMyPlayer()->getMaxHp());
		//does pathfinding on click and player collision
		pathfinding->update();

		//pass collidbales to projectile management every update so that projectiles can even be stopped by moving terrain
		auto collidables = worldHandling->terrain->getCollidables();
		projectileHandling->update(collidables->data(), collidables->size());

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
		playerHandling->draw();
		projectileHandling->draw();//draw after players
		uiHandling->draw();//draw last, should be over every item ingame
	}
}


//Neworking part-----------------------------------------------------------------------------------------------------------------


void initServer() {
	server = new GameServer();
	server->bindMutex(nMutex);
	server->waitForClient();
}

void initClient() {
	std::string s = "192.168.178.28";//TODO: typeable ip
	client = new GameClient(s.c_str());
	client->bindMutex(nMutex);
	client->receive();
}


void sendData() {
	NetworkCommunication::initNewCommunication();

	playerHandling->sendPlayerData();
	projectileHandling->sendProjectiles();

	if (playerHandling->getPlayerIndex() == 0) {
		NetworkCommunication::sendTokensToServer(server);
	}
	if (playerHandling->getPlayerIndex() == 1) {
		NetworkCommunication::sendTokensToClient(client);
	}
}

void recvAndImplementData() {

	bool receivedSth = false;
	if (playerHandling->getPlayerIndex() == 0) {
		if (server->newMessage() == true) {
			receivedSth = true;
			NetworkCommunication::receiveTonkensFromServer(server);
		}
	}
	else {
		if (client->newMessage() == true) {
			receivedSth = true;
			NetworkCommunication::receiveTonkensFromClient(client);
		}
	}
	if (receivedSth == true) {
		playerHandling->receivePlayerData(pathfinding);
		projectileHandling->receiveProjectiles();
		received = true;
	}
}
