#include "GlobalRecources.hpp"
#include <mutex>
#include "iostream" 
using namespace std;

int GlobalRecources::worldWidth, GlobalRecources::worldHeight;
int GlobalRecources::frameWidth, GlobalRecources::frameHeight;
shared_ptr<Terrain> GlobalRecources::terrain;
shared_ptr<vector<shared_ptr<Player>>> GlobalRecources::players;
Pathfinding* GlobalRecources::pFinding;
shared_ptr<mutex> GlobalRecources::pfMtx;
bool GlobalRecources::isServer = false;
shared_ptr<vector<bool>> GlobalRecources::initNetwork = shared_ptr<vector<bool>>(new vector<bool>());