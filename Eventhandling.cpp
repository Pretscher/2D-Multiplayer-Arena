#include "Eventhandling.hpp"
#include "Renderer.hpp"
#include "Player.hpp"
#include "aStarAlgorithm/Graph.hpp"
#include "aStarAlgorithm/Algorithm.hpp"
#include "Terrain.hpp"
#include "Projectile.hpp"

#include <thread>
#include <mutex>

static Player* player;//movable player
static Terrain* terrain;//contains every non-moving object with collision

//Viewspace: value of 0 means left/top, limit value (from viewSpaceLimits) means right/bottom
static int* cViewSpace;//current viewspace position. [0] is row (bot to top), [1] is col (left to right) 
static int* viewSpaceLimits;//maximum viewspace.  [0] = left, [1] = right, [2] = top, [3] = bottom

static int rows, cols;//rows and cols you can see at a time, viewspace limits need to be added for worldSize
static int pathfindingAccuracy;//the higher the less accuracy (1 means every pixel is considered)

//double array for every visitable (pathfindingaccuracy factored in) pixel, 
//if it can be used for pathfinding true, else false
static bool** collisionGrid;


static std::vector<Projectile*>* projectiles;//stores all projectiles for creation, drawing, moving and damage calculation. 

//forward declarations
static void pathFindingOnClick();
static void pathFindingInit();
static void hardCodeTerrain();
static void projectileManagement();

static Graph* g;

void eventhandling::init() {
	player = new Player(cols / 2, rows / 2, 100, 100, 0.5f);

	terrain = new Terrain();
	hardCodeTerrain();

	pathFindingInit();
	projectiles = new std::vector<Projectile*>();
	Renderer::linkViewSpace(cViewSpace, viewSpaceLimits);

}

void eventhandling::eventloop() {
	Renderer::updateViewSpace();//move view space if mouse on edge of window
	pathFindingOnClick();//right click => find path to right clicked spot and give it to player
	projectileManagement();
	player->move();//if he has a path, he moves on this path
}

void eventhandling::drawingloop() {
	terrain->draw();
	player->draw();
	for (int i = 0; i < projectiles->size(); i++) {
		projectiles->at(i)->draw();
	}
}



//Pathfinding--------------------------------------------------------------------------------------------

static std::thread* pathFindingThread;
static std::mutex* finishedPathfinding;
//rows and cols of one visible window + viewspace limit coords = max coords we need for pathfinding
static int worldRows;
static int worldCols;
static void pathFindingInit() {
	pathFindingThread = nullptr;
	finishedPathfinding = new std::mutex();
	viewSpaceLimits = new int[4];
	viewSpaceLimits[0] = 0;//left
	viewSpaceLimits[1] = 2000;//right
	viewSpaceLimits[2] = 0;//top
	viewSpaceLimits[3] = 2000;//bot
	cViewSpace = new int[2];
	cViewSpace[0] = 0;//row (top to bot)
	cViewSpace[1] = 0;//col (left to right)

	//hardcoded rows and cols
	rows = 1080;
	cols = 1920;
	worldRows = rows + viewSpaceLimits[3];
	worldCols = cols + viewSpaceLimits[1];

	pathfindingAccuracy = 20;
	//rows and cols are stretched to full screen anyway. Its just accuracy of rendering 
	//and relative coords on which you can orient your drawing. Also makes drawing a rect
	//and stuff easy because width can equal height to make it have sides of the same lenght.
	Renderer::initGrid(rows, cols);

	int pathfindingRows = worldRows / pathfindingAccuracy;//max rows for pathfinding
	int pathfindingCols = worldCols / pathfindingAccuracy;//max cols for pathfinding
	collisionGrid = new bool* [pathfindingRows];
	for (int y = 0; y < pathfindingRows; y++) {
		collisionGrid[y] = new bool[pathfindingCols];
		for (int x = 0; x < pathfindingCols; x++) {
			collisionGrid[y][x] = true;
		}
	}

	//create graph from unmoving solids, can be changed dynamically
	terrain->addCollidablesToGrid(collisionGrid, pathfindingAccuracy, player->getDrawWidth(), player->getDrawHeight());
	g = new Graph(worldRows / pathfindingAccuracy, worldCols / pathfindingAccuracy);
	g->generateWorldGraph(collisionGrid);
}

static void startPathFinding(int mouseX, int mouseY);
static bool sameClick = false;//dont do two pathfindings on the same click
static bool findingPath = false;
static void pathFindingOnClick() {
	if (sf::Mouse::isButtonPressed(sf::Mouse::Right) == false){
		sameClick = false;
	}

	//if rightclick is pressed, find path from player to position of click
	finishedPathfinding->lock();
	if (sf::Mouse::isButtonPressed(sf::Mouse::Right) == true && sameClick == false && findingPath == false) {
		sameClick = true;

		player->deletePath();
		int mouseX = -1, mouseY = -1;
		Renderer::getMousePos(&mouseX, &mouseY, true);//writes mouse coords into mouseX, mouseY
		if (mouseX != -1) {//stays at -1 if click is outside of window
			//if (pathFindingThread != nullptr) delete pathFindingThread;
			findingPath = true;
			pathFindingThread = new std::thread(&startPathFinding, mouseX, mouseY);
		}
	}
	finishedPathfinding->unlock();
}

static void startPathFinding(int mouseX, int mouseY) {

	int* xPath = nullptr;
	int* yPath = nullptr;
	int pathlenght = 0;

	int mouseRow = mouseY / pathfindingAccuracy;
	int mouseCol = mouseX / pathfindingAccuracy;

	Algorithm::findPath(&xPath, &yPath, &pathlenght, g, player->getRow() / pathfindingAccuracy, player->getCol() / pathfindingAccuracy, mouseRow, mouseCol);
	//reverse accuracy simplification
	for (int i = 0; i < pathlenght; i++) {
		xPath[i] *= pathfindingAccuracy;
		yPath[i] *= pathfindingAccuracy;
	}

	finishedPathfinding->lock();
	player->givePath(xPath, yPath, pathlenght);
	findingPath = false;
	finishedPathfinding->unlock();
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

		int row = 0, col = 0;
		int halfW = player->getDrawWidth() / 2;
		int halfH = player->getDrawHeight() / 2;


		//if projectile distination is above player
		if (mouseY < player->getRow()) {
			col = player->getCol() + halfW;
			row = player->getRow();
			player->setTexture(2);
		}
		//below
		if (mouseY > player->getRow()) {
			col = player->getCol() + halfW;
			row = player->getRow() + player->getDrawHeight();
			player->setTexture(3);
		}



		Projectile* p = new Projectile(row, col, velocity, mouseY, mouseX, 20);
		projectiles->push_back(p);
	}

	//move projectiles (we loop through em in drawingLoop too but later it will be in a different thread so we cant use the same loop)
	for (int i = 0; i < projectiles->size(); i++) {
		Projectile* p = projectiles->at(i);
		p->move(worldRows, worldCols, terrain->getCollidables()->data(), terrain->getCollidables()->size());//give it the maximum rows so it know when it can stop moving
		if (p->isDead() == true) {
			projectiles->erase(projectiles->begin() + i);//delete projecile if dead
		}
	}
}
