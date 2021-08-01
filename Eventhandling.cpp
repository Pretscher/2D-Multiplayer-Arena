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

static Player** players;//all players
static int myPlayerI;
static int playerCount;//number of players
static int worldRows;
static int worldCols;
static Terrain* terrain;//contains every non-moving object with collision

//Viewspace: value of 0 means left/top, limit value (from viewSpaceLimits) means right/bottom
static int* cViewSpace;//current viewspace position. [0] is row (bot to top), [1] is col (left to right) 
static int* viewSpaceLimits;//maximum viewspace.  [0] = left, [1] = right, [2] = top, [3] = bottom

static int rows, cols;//rows and cols you can see at a time, viewspace limits need to be added for worldSize
static Pathfinding* pathfinding;
static std::vector<Projectile*>* projectiles;//stores all projectiles for creation, drawing, moving and damage calculation. 

static int uiHeight;
static Menu* menu;

static GameServer* server;
static GameClient* client;

//forward declarations
static void hardCodeTerrain();
static void projectileManagement();

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
static bool isServer;
static std::thread* networkThread;
void eventhandling::init() {
	menu = new Menu();

	viewSpaceLimits = new int[4];
	viewSpaceLimits[0] = 0;//left
	viewSpaceLimits[1] = 2000;//right
	viewSpaceLimits[2] = 0;//top
	viewSpaceLimits[3] = 2000;//bot
	cViewSpace = new int[2];
	cViewSpace[0] = 0;//row (top to bot)
	cViewSpace[1] = 0;//col (left to right)

	uiHeight = 900;

	//hardcoded rows and cols
	rows = 1080;
	cols = 1920;
	worldRows = rows + viewSpaceLimits[3];
	worldCols = cols + viewSpaceLimits[1];

	Renderer::limitMouse(uiHeight, cols);
	Renderer::initGrid(rows, cols);
	Renderer::linkViewSpace(cViewSpace, viewSpaceLimits);

	initPlayers();
	terrain = new Terrain();
	hardCodeTerrain();

	pathfinding = new Pathfinding(worldRows, worldCols, terrain, players, playerCount);

	projectiles = new std::vector<Projectile*>();
}

bool direction = false;
bool menuActive = true;
static void passPositions();
static void implementPositions();
void eventhandling::eventloop() {
	if (menuActive == true) {
		menu->update();
		if (menu->hostServer() == true) {
			networkThread = new std::thread(&initServer);
			isServer = true;
			menuActive = false;
		}
		if (menu->connectAsClient() == true) {
			networkThread = new std::thread(&initClient);
			isServer = false;
			menuActive = false;
		}
	}




	if (menuActive == false) {

		Renderer::updateViewSpace();//move view space if mouse on edge of window
		pathfinding->pathFindingOnClick(myPlayerI);//right click => find path to right clicked spot and give it to player
		pathfinding->moveObjects();
		projectileManagement();

		if (pathfinding->isPlayerUseable() == true) {
			if (players[1]->hasPath() == false && players[1]->isFindingPath() == false) {
				if (direction == false) {
					pathfinding->findPath(1000, 1000, 1);
					direction = true;
				}
				else {
					pathfinding->findPath(100, 100, 1);
					direction = false;
				}
			}
		}
		if (server != nullptr || client != nullptr) {
			passPositions();
			implementPositions();
		}
	}

}

static void drawUI() {

	Renderer::drawRect(uiHeight, 0, cols, rows - uiHeight, sf::Color(50, 50, 50, 255), true);
	Renderer::drawRect(uiHeight + 50, 50, (cols - 100), 50, sf::Color(10, 10, 10, 255), true);
	if (players[myPlayerI]->getHp() > 0) {
		float widthMult = (float)players[myPlayerI]->getHp() / players[myPlayerI]->getMaxHp();
		float width = ((float)cols - 100.0f) * widthMult;
		Renderer::drawRect(uiHeight + 50, 50, width, 50, sf::Color(0, 150, 0, 255), true);
	}
}

void eventhandling::drawingloop() {
	if (menuActive == true) {
		menu->drawMenu();
	}
	else {
		terrain->draw();
		for (int i = 0; i < playerCount; i++) {
			if (players[i]->getHp() > 0) {
				players[i]->draw();//if he has a path, he moves on this path
			}
		}
		for (int i = 0; i < projectiles->size(); i++) {
			projectiles->at(i)->draw();
		}
		drawUI();
	}
}



//Game Object initialization

static void hardCodeTerrain() {
	terrain->addRect(1000, 1000, 500, 200);
	terrain->addRect(200, 200, 500, 200);
	terrain->addRect(1000, 1000, 500, 200);
}

static bool samePress = false;
static void projectileManagement() {
	//dont shoot a projectile for the same space-press
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) == false) {
		samePress = false;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) == true && samePress == false) {
  		samePress = true;

		float velocity = 10.0f;
		int mouseX = -1, mouseY = -1;
		Renderer::getMousePos(&mouseX, &mouseY, true);//writes mouse coords into mouseX, mouseY
		//calculates a function between these points and moves on it

		Player* myPlayer = players[myPlayerI];

		int row = 0, col = 0;
		int halfW = myPlayer->getWidth() / 2;
		int halfH = myPlayer->getHeight() / 2;


		//if projectile distination is above player
		if (mouseY < myPlayer->getRow()) {
			col = myPlayer->getCol() + halfW;
			row = myPlayer->getRow();
			myPlayer->setTexture(2);
		}
		//below
		if (mouseY > myPlayer->getRow()) {
			col = myPlayer->getCol() + halfW;
			row = myPlayer->getRow() + myPlayer->getHeight();
			myPlayer->setTexture(3);
		}



		Projectile* p = new Projectile(row, col, velocity, mouseY, mouseX, 20, myPlayer);
		projectiles->push_back(p);
	}

	//move projectiles (we loop through em in drawingLoop too but later it will be in a different thread so we cant use the same loop)
	for (int i = 0; i < projectiles->size(); i++) {
		Projectile* p = projectiles->at(i);
		p->move(worldRows, worldCols, terrain->getCollidables()->data(), terrain->getCollidables()->size());//give it the maximum rows so it know when it can stop moving
		
		for (int j = 0; j < playerCount; j++) {
			Player* cPlayer = players[j];
			if (cPlayer != p->getPlayer()) {
				if (players[j]->getHp() > 0) {

					if (Utils::collisionRectCircle(cPlayer->getRow(), cPlayer->getCol(), cPlayer->getWidth(), cPlayer->getHeight(),
						p->getRow(), p->getCol(), p->getRadius(), 10) == true) {
						p->setDead(true);
						cPlayer->setHp(cPlayer->getHp() - p->getPlayer()->getDmg());
					}
				}
			}
		}
		
		if (p->isDead() == true) {
			projectiles->erase(projectiles->begin() + i);//delete projecile if dead
		}
	}
}

static void passPositions() {
	std::string* positions = new std::string();
	positions->push_back(players[myPlayerI]->getRow());
	positions->push_back(',');
	positions->push_back(players[myPlayerI]->getCol());

	if (myPlayerI == 0) {
		server->sendToClient(positions->c_str());
	}
	if (myPlayerI == 1) {
		client->sendToServer(positions->c_str());
	}
	delete positions;
}

std::vector<int>* extractInts(std::string* str) {
	std::vector<int>* out = new std::vector<int>();
	int lastSplit = 0;
	for (int i = 0; i < str->length(); i++) {
		if (str->at(i) == ',') {
			int temp = std::stoi(str->substr(lastSplit, i));
			out->push_back(temp);
			lastSplit = i;
		}
	}
	return out;
}

static void implementPositions() {
	std::string* positions;
	if (myPlayerI == 0) {
		positions = server->getLastMessage();
		if (positions != nullptr) {
			std::vector<int>* intPositions = extractInts(positions);
			players[1]->setRow(positions->at(0));
			players[1]->setCol(positions->at(1));
		}
	}
	if (myPlayerI == 1) {
		positions = client->getLastMessage();
		if (positions != nullptr) {
			std::vector<int>* intPositions = extractInts(positions);
			players[0]->setRow(positions->at(0));
			players[0]->setCol(positions->at(1));
		}
	}
}

static void initServer() {
	server = new GameServer();
	server->waitForClient();
	myPlayerI = 0;
}

static void initClient() {
	std::string s = "192.168.178.28";
	client = new GameClient(s.c_str());
	client->sendToServer("hi there");
	client->receive();
	myPlayerI = 1;
}

