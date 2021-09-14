#include "GlobalRecources.hpp"
#include <mutex>

int GlobalRecources::worldWidth, GlobalRecources::worldHeight;
int GlobalRecources::frameWidth, GlobalRecources::frameHeight;
Terrain* GlobalRecources::terrain;
Player** GlobalRecources::players;
int GlobalRecources::playerCount;
Pathfinding* GlobalRecources::pFinding;
std::mutex* GlobalRecources::pfMtx;