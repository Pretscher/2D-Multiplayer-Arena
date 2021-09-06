#include "GlobalRecources.hpp"
#include <mutex>

int GlobalRecources::worldRows;
int GlobalRecources:: worldCols;
Terrain* GlobalRecources::terrain;
Player** GlobalRecources::players;
int GlobalRecources::playerCount;
Pathfinding* GlobalRecources::pFinding;
std::mutex* GlobalRecources::pfMtx;

void GlobalRecources::init(Player** i_players, int i_playerCount, Terrain* i_terrain, int i_worldRows, int i_worldCols, Pathfinding* i_pathfinding, std::mutex* i_pfMtx) {
    players = i_players;
    playerCount = i_playerCount;
    terrain = i_terrain;
    worldRows = i_worldRows;
    worldCols = i_worldCols;
    pFinding = i_pathfinding;
    pfMtx = i_pfMtx;
}
