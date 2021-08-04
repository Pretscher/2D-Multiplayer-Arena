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


static Player** players;//all players
static int myPlayerI;
static int playerCount;//number of players

static Pathfinding* pathfinding;


static int uiHeight;
static Menu* menu;

static GameServer* server;
static GameClient* client;

static UiHandling* uiHandling;
static WorldHandling* worldHandling;
static ProjectileHandling* projectileHandling;


static void initPlayers() {
	playerCount = 2;

	float stepsPerIteration = 0.5f;//velocity on path if path is given
	players = new Player * [playerCount];
	for (int i = 0; i < playerCount; i++) {
		players[i] = new Player(500, i * 1000 / playerCount, 100, 100, stepsPerIteration, 200, 10);//places players on map, col dist depends on playercount
	}
	myPlayerI = 0;
}

static void initServer();
static void initClient();
static std::thread* networkThread;
void eventhandling::init() {
	initPlayers();
	worldHandling = new WorldHandling();
	uiHandling = new UiHandling(worldHandling->frameRows, worldHandling->frameCols);
	pathfinding = new Pathfinding(worldHandling->worldRows, worldHandling->worldCols, worldHandling->terrain, players, playerCount, myPlayerI);
	projectileHandling = new ProjectileHandling(worldHandling->worldRows, worldHandling->worldCols, players, myPlayerI, playerCount);


	menu = new Menu();
}

bool direction = false;
bool menuActive = true;
static void passPositions();
static void implementPositions();


void eventhandling::drawingloop() {
	if (menuActive == true) {
		menu->drawMenu();
	}
	else {
		worldHandling->draw();//draw first, lifebars and stuff should be drawn over it

		for (int i = 0; i < playerCount; i++) {
			if (players[i]->getHp() > 0) {
				players[i]->draw();//if he has a path, he moves on this path
			}
		}
		projectileHandling->draw();//draw after players
		uiHandling->draw();//draw last, should be over every item ingame
	}
}



//Game Object initialization






bool connectionSetUp = false;
std::mutex* nMutex = new std::mutex();
static void passPositions() {
	std::string* data = new std::string();

	data->append(std::to_string(players[myPlayerI]->getRow()).c_str());
	data->push_back(',');
	data->append(std::to_string(players[myPlayerI]->getCol()).c_str());
	data->push_back(',');
	data->append(std::to_string(players[myPlayerI]->getTextureIndex()));
	data->push_back(',');
	data->append(std::to_string(players[myPlayerI]->getHp()));

	projectileHandling->sendProjectiles(data);

	if (myPlayerI == 0) {
		server->sendToClient(data->c_str());
	}
	if (myPlayerI == 1) {
		client->sendToServer(data->c_str());
	}
	delete data;
}

std::vector<int>* extractInts(std::string* str) {
	std::vector<int>* out = new std::vector<int>();
	int lastSplit = 0;
	for (int i = 0; i < str->length(); i++) {
		if (str->at(i) == ',') {
			int temp = std::stoi(str->substr(lastSplit, i));
			out->push_back(temp);
			lastSplit = i + 1;
		}
		if (i == str->length() - 1) {
			int temp = std::stoi(str->substr(lastSplit, i));
			out->push_back(temp);
		}
	}
	return out;
}


static void implementPositions() {
	std::string* data;
	int otherPlayer;
	bool copyAndParse = false;
	nMutex->lock();//gets locked before writing message
	if (myPlayerI == 0) {
		otherPlayer = 1;
		data = server->getLastMessage();
		if (data != nullptr && data->size() > 0) {
			copyAndParse = true;
		}
	}
	else {
		otherPlayer = 0;
		data = client->getLastMessage();
		if (data != nullptr && data->size() > 0) {
			copyAndParse = true;
		}
	}
	if (copyAndParse == true) {
		data = new std::string(data->c_str());//copy so network can overwrite
		nMutex->unlock();
		std::vector<int>* parseToIntsData = extractInts(data);

		int tempRow = players[otherPlayer]->getRow();
		int tempCol = players[otherPlayer]->getCol();
		players[otherPlayer]->setRow(parseToIntsData->at(0));
		players[otherPlayer]->setCol(parseToIntsData->at(1));
		players[otherPlayer]->setTexture(parseToIntsData->at(2));
		players[otherPlayer]->setHp(parseToIntsData->at(3));

		projectileHandling->receiveProjectiles(parseToIntsData);
		
		pathfinding->enableArea(tempRow, tempCol, players[0]->getWidth() + 100, players[0]->getHeight() + 100);//enable old position
		pathfinding->disableArea(players[otherPlayer]->getRow(), players[otherPlayer]->getCol(),
			players[0]->getWidth(), players[0]->getHeight());//disable new position
		//could have enabled other players position aswell
		pathfinding->disableArea(players[myPlayerI]->getRow(), players[myPlayerI]->getCol(), players[0]->getWidth(), players[0]->getHeight());//disable new position
		
		delete data;
		delete parseToIntsData;
	}
	else {//the earlier you unlock the better
		nMutex->unlock();
	}
}

static void initServer() {
	server = new GameServer();
	server->bindMutex(nMutex);
	server->waitForClient();
}

static void initClient() {
	std::string s = "192.168.178.28";//TODO: typeable ip
	client = new GameClient(s.c_str());
	client->bindMutex(nMutex);
	client->receive();
}


void eventhandling::eventloop() {
	//if host or client has not been selected, wait for it to happen
	if (menuActive == true) {
		menu->update();
		if (menu->hostServer() == true) {
			myPlayerI = 0;//server has player index 0
			networkThread = new std::thread(&initServer);
			menuActive = false;//go to game
		}
		if (menu->connectAsClient() == true) {
			myPlayerI = 1;//right now there are only two players so the client just has index 1
			networkThread = new std::thread(&initClient);
			menuActive = false;//go to game
		}
	}

	//if host or client has been selected
	if (menuActive == false) {
		worldHandling->update();
		//pass current hp informations to uiHandling so that it can draw a proper life bar
		uiHandling->updateLifeBar(players[myPlayerI]->getHp(), players[myPlayerI]->getMaxHp());
		//does pathfinding on click and player collision
		pathfinding->update();

		//pass collidbales to projectile management every update so that projectiles can even be stopped by moving terrain
		auto collidables = worldHandling->terrain->getCollidables();
		projectileHandling->update(collidables->data(), collidables->size());
		
		//pass game information back and forth through tcp sockets
		if ((server != nullptr && server->isConnected() == true) || (client != nullptr && client->isConnected() == true)) {
			passPositions();
			implementPositions();
		}
	}
}

