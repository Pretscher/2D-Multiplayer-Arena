#include "Eventhandling.hpp"
using namespace std;



void initServer(shared_ptr<PortableServer> server) {
	server = shared_ptr<PortableServer>(new PortableServer());
	server->waitForClient();
	server->receiveMultithreaded();
}

void initClient(shared_ptr<PortableClient> client) {
	string s = "192.168.178.28";//TODO: typeable ip
	client = shared_ptr<PortableClient>(new PortableClient(s.c_str()));
	client->waitForServer();
	client->receiveMultithreaded();
}

Eventhandling::Eventhandling() {
	int worldWidth = 1920, worldHeight = 1080, vsWidth = 2000, vsHeight = 2000;
	//be sure to not change the order, they depend on each other heavily
	playerHandling = unique_ptr<PlayerHandling>(new PlayerHandling());
	worldHandling = unique_ptr<WorldHandling>(new WorldHandling(worldWidth, worldHeight, vsWidth, vsHeight));
	uiHandling = unique_ptr<UiHandling>(new UiHandling());
	pathfinding = unique_ptr<Pathfinding>(new Pathfinding());
	projectileHandling = unique_ptr<ProjectileHandling>(new ProjectileHandling());
	menu = unique_ptr<Menu>(new Menu());
}

void Eventhandling::eventloop() {
	//if host or client has not been selected, wait for it to happen
	if (menuActive == true) {
		menu->update();
		bool initIndex = false;
		int playerIndex;
		if (menu->hostServer() == true) {
			playerIndex = 0;
			initIndex = true;
			networkThread = new thread(&initServer, server);
			menuActive = false;//go to game
		}
		if (menu->connectAsClient() == true) {
			playerIndex = 1;
			initIndex = true;
			networkThread = new thread(&initClient, client);
			menuActive = false;//go to game
		}
		if (initIndex == true) {
			playerHandling->setPlayerIndex(playerIndex);//right now there are only two players so the client just has index 1
			pathfinding->setPlayerIndex(playerIndex);
			projectileHandling->setPlayerIndex(playerIndex);
			abilityHandling = unique_ptr<AbilityHandling>(new AbilityHandling(playerIndex));
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
		projectileHandling->update(worldHandling->getTerrain()->getCollidables());

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


void Eventhandling::drawingloop() {
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



void Eventhandling::sendData() {
	NetworkCommunication::initNewCommunication();

	abilityHandling->sendData();
	playerHandling->sendPlayerData();
	projectileHandling->sendProjectiles();


	if (playerHandling->getPlayerIndex() == 0) {
		NetworkCommunication::sendTokensToServer(server);
	}
	if (playerHandling->getPlayerIndex() == 1) {
		NetworkCommunication::sendTokensToClient(client);
	}
}

void Eventhandling::recvAndImplementData() {

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
		abilityHandling->receiveData();
		playerHandling->receivePlayerData();
		projectileHandling->receiveProjectiles();

		received = true;
	}
}
