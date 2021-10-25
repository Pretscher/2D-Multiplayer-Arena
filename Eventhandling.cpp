#include "Eventhandling.hpp"
using namespace std;

bool networkInitialized = false;
bool isClient = false;
shared_ptr<PortableServer> server = nullptr;
shared_ptr<PortableClient> client = nullptr;
void initServer() {
	while (true) {
		server = shared_ptr<PortableServer>(new PortableServer());
		server->waitForClient();
	}
}

vector<string> availableHosts;

void initClient() {
	//string ip = "192.168.178.28";//TODO: typeable ip
	client = shared_ptr<PortableClient>(new PortableClient());
	client->searchHosts();
	while (true) {
		this_thread::sleep_for(chrono::milliseconds(1));
		if (client->isConnected() == true) {
			client->receiveMultithreaded();
		}
	}
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

static bool triesToConnect = false;
void Eventhandling::eventloop() {
	//if host or client has not been selected, wait for it to happen
	if (menuActive == true) {
		menu->update();
		bool initIndex = false;
		int playerIndex;
		if (triesToConnect == false) {
			if (menu->hostServer() == true) {
				playerIndex = 0;
				initIndex = true;
				networkThread = new thread(&initServer);
				menuActive = false;//go to game
				triesToConnect = true;
				menu->selectHostMenu();
			}
			if (menu->connectAsClient() == true) {
				playerIndex = 1;
				initIndex = true;
				networkThread = new thread(&initClient);
				triesToConnect = true;
				menu->selectHostMenu();
			}
		}
		if (initIndex == true) {
			playerHandling->setPlayerIndex(playerIndex);//right now there are only two players so the client just has index 1
			pathfinding->setPlayerIndex(playerIndex);
			projectileHandling->setPlayerIndex(playerIndex);
			abilityHandling = unique_ptr<AbilityHandling>(new AbilityHandling(playerIndex));
		}

		if (client != nullptr) {
			if (triesToConnect == true && menu->connectAsClient() == true) {
				//host selection
				availableHosts = client->getAvailableHosts();//shown in menu
				menu->giveAvailableHosts(std::move(availableHosts));
				if (menu->getSelectedHost() != nullptr) {
					client->connectToHost(*menu->getSelectedHost());
					menuActive = false;//go to game
				}
			}
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
		if (server != nullptr && server->getClientCount() > 0) {
			while (received.size() < server->getClientCount()) {
				received.push_back(false);
				playerHandling->getPlayers()->push_back(shared_ptr<Player>(new Player()));
				isClient = false;//ye i know this is set to false multiple times if there are more than 1 con but i cant be bothered
			}
			for (int i = 0; i < server->getClientCount(); i++) {
				if (received[i] == true) {//handshaking: only if something was received send again. Prevents lag and unwanted behavior
					sendData(i);
					received[i] = false;
				}
				recvAndImplementData(i);
			}
		}
		if (client != nullptr && client->isConnected() == true) {
			if (networkInitialized == false) {
				networkInitialized = true;
				received.push_back(true);
				isClient = true;
			}
			received.push_back(true);
			if (received[0] == true) {//handshaking: only if something was received send again. Prevents lag and unwanted behavior
				sendData(0);
				received[0] = false;
			}
			recvAndImplementData(0);
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

	for (int i = 0; i < availableHosts.size(); i++) {
		Renderer::drawRect(400, 200 + (i * 250), 200, 100, sf::Color(255, 0, 255, 255), true);
	}
	
}


//Neworking part-----------------------------------------------------------------------------------------------------------------



void Eventhandling::sendData(int index) {
	NetworkCommunication::initNewCommunication(index);
	abilityHandling->sendData();
	playerHandling->sendPlayerData();
	projectileHandling->sendProjectiles(index);
	if (isClient == true) {
		NetworkCommunication::sendTokensToClient(client);
	}
	else {
		NetworkCommunication::sendTokensToServer(index, server);
	}
}

void Eventhandling::recvAndImplementData(int index) {
	NetworkCommunication::initNewCommunication(index);
	if (isClient == true) {
		if (client->newMessage() == true) {
			NetworkCommunication::receiveTonkensFromClient(client);
			abilityHandling->receiveData(index);
			playerHandling->receivePlayerData(index);
			projectileHandling->receiveProjectiles(index);
			received[index] = true;
		}
	}
	else {
		if (server->newMessage(index) == true) {
			NetworkCommunication::receiveTonkensFromServer(index, server);
			abilityHandling->receiveData(index);
			playerHandling->receivePlayerData(index);
			projectileHandling->receiveProjectiles(index);
			received[index] = true;
		}
	}

}
