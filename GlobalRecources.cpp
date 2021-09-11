#include "GlobalRecources.hpp"
#include <mutex>

int GlobalRecources::worldYs;
int GlobalRecources:: worldXs;
Terrain* GlobalRecources::terrain;
Player** GlobalRecources::players;
int GlobalRecources::playerCount;
Pathfinding* GlobalRecources::pFinding;
std::mutex* GlobalRecources::pfMtx;

void GlobalRecources::init(Player** i_players, int i_playerCount, Terrain* i_terrain, int i_worldYs, int i_worldXs, Pathfinding* i_pathfinding, std::mutex* i_pfMtx) {
    players = i_players;
    playerCount = i_playerCount;
    terrain = i_terrain;
    worldYs = i_worldYs;
    worldXs = i_worldXs;
    pFinding = i_pathfinding;
    pfMtx = i_pfMtx;
}
