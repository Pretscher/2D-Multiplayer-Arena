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

float projectileVel = 10.0f;
int projectileRadius = 20;

std::vector<Projectile*>* newProjectiles = new std::vector<Projectile*>();;
static bool samePress = false;
static void projectileManagement() {
	//dont shoot a projectile for the same space-press
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) == false) {
		samePress = false;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) == true && samePress == false) {
  		samePress = true;

		
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



		Projectile* p = new Projectile(row, col, projectileVel, mouseY, mouseX, projectileRadius, myPlayer);
		projectiles->push_back(p);
		newProjectiles->push_back(p);
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


bool connectionSetUp = false;
std::mutex* nMutex = new std::mutex();
static void passPositions() {
	std::string* positions = new std::string();

	positions->append(std::to_string(players[myPlayerI]->getRow()));
	positions->push_back(',');
	positions->append(std::to_string(players[myPlayerI]->getCol()));
	positions->push_back(',');
	positions->append(std::to_string(players[myPlayerI]->getTextureIndex()));
	positions->push_back(',');
	positions->append(std::to_string(players[myPlayerI]->getHp()));
	positions->push_back(',');
	positions->append(std::to_string(newProjectiles->size() * 4));//projectile size in string (4 values per proj)
	positions->push_back(',');
	positions->append(std::to_string(pathfinding->newGoalRows->size() * 2));//algorithm goal size in string (2 values per path)

	for (int i = 0; i < newProjectiles->size(); i++) {
		positions->push_back(',');
		positions->append(std::to_string((int)newProjectiles->at(i)->getRow()));
		positions->push_back(',');
		positions->append(std::to_string((int)newProjectiles->at(i)->getCol()));
		positions->push_back(',');
		positions->append(std::to_string(newProjectiles->at(i)->getGoalRow()));
		positions->push_back(',');
		positions->append(std::to_string(newProjectiles->at(i)->getGoalCol()));
	}

	for (int i = 0; i < pathfinding->newGoalRows->size(); i++) {
		positions->push_back(',');
		positions->append(std::to_string(pathfinding->newGoalRows->at(i)));
		positions->push_back(',');
		positions->append(std::to_string(pathfinding->newGoalCols->at(i)));
	}
	newProjectiles->clear();
	pathfinding->newGoalRows->clear();
	pathfinding->newGoalCols->clear();

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
	std::string* msg;
	int otherPlayer;
	bool copyAndParse = false;
	nMutex->lock();//gets locked before writing message
	if (myPlayerI == 0) {
		otherPlayer = 1;
		msg = server->getLastMessage();
		if (msg != nullptr && msg->size() > 0) {
			copyAndParse = true;
		}
	}
	else {
		otherPlayer = 0;
		msg = client->getLastMessage();
		if (msg != nullptr && msg->size() > 0) {
			copyAndParse = true;
		}
	}
	if (copyAndParse == true) {
		//we dont want to bother the network anymore, copy it and dont use the pointer from now on
		msg = new std::string(msg->c_str());
		nMutex->unlock();
		std::vector<int>* intPositions = extractInts(msg);

		int tempRow = players[otherPlayer]->getRow();
		int tempCol = players[otherPlayer]->getCol();
		players[otherPlayer]->setTexture(intPositions->at(2));
		players[otherPlayer]->setHp(intPositions->at(3));

		int counter = 0;
		int row = 0;
		int col = 0;
		int goalRow = 0;
		int goalCol = 0;
		int projectileLenghtInString = intPositions->at(4);
		for (int i = 6; i < projectileLenghtInString; i++) {
			switch (counter) {
			case 0:
				row = intPositions->at(i);
			case 1:
				col = intPositions->at(i);
			case 2:
				goalRow = intPositions->at(i);//keep important decimal places through */ 10000
			case 3:
				goalCol = intPositions->at(i);
			default:;//do nothing on default, either all 4 cases are given or none, nothing else can happen
			}
			counter++;
			if (counter > 3) {
				projectiles->push_back(new Projectile(row, col, projectileVel, goalRow, goalCol, 
					projectileRadius, players[otherPlayer]));
				counter = 0;
			}
		}
		
		int startIndex = 6 + projectileLenghtInString;

		int pfCounter = 0;
		int pfRow = 0;
		int pfCol = 0;
		for (int i = startIndex; i < startIndex + intPositions->at(5); i++) {
			switch (counter) {
			case 0:
				pfRow = intPositions->at(i);
			case 1:
				pfCol = intPositions->at(i);
			default:;//do nothing on default, either all 4 cases are given or none, nothing else can happen
			}

			counter++;
			if (counter > 1) {
				pathfinding->findPath(pfCol, pfRow, otherPlayer);
				counter = 0;
			}
		}

		delete msg;
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
	std::string s = "192.168.178.28";
	client = new GameClient(s.c_str());
	client->bindMutex(nMutex);
	client->receive();
}


void eventhandling::eventloop() {
	if (menuActive == true) {
		menu->update();
		if (menu->hostServer() == true) {
			myPlayerI = 0;
			networkThread = new std::thread(&initServer);
			menuActive = false;
		}
		if (menu->connectAsClient() == true) {
			myPlayerI = 1;
			networkThread = new std::thread(&initClient);
			menuActive = false;
		}
		pathfinding->setPlayerIndex(myPlayerI);
	}




	if (menuActive == false) {

		Renderer::updateViewSpace();//move view space if mouse on edge of window
		pathfinding->pathFindingOnClick(myPlayerI);//right click => find path to right clicked spot and give it to player
		pathfinding->moveObjects();
		projectileManagement();
		if ((server != nullptr && server->isConnected() == true) || (client != nullptr && client->isConnected() == true)) {
			passPositions();
			implementPositions();
		}
	}
}

