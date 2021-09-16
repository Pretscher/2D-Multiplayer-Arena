#include "GlobalRecources.hpp"
#include <mutex>
#include "iostream" 
using namespace std;

int GlobalRecources::worldWidth, GlobalRecources::worldHeight;
int GlobalRecources::frameWidth, GlobalRecources::frameHeight;
Terrain* GlobalRecources::terrain;
Player** GlobalRecources::players;
int GlobalRecources::playerCount;
Pathfinding* GlobalRecources::pFinding;
mutex* GlobalRecources::pfMtx;