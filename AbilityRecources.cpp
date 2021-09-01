#include "AbilityRecources.hpp"

int AbilityRecources::worldRows;
int AbilityRecources:: worldCols;
Terrain* AbilityRecources::terrain;
Player** AbilityRecources::players;
int AbilityRecources::playerCount;
Pathfinding* AbilityRecources::pFinding;

void AbilityRecources::init(Player** i_players, int i_playerCount, Terrain* i_terrain, int i_worldRows, int i_worldCols, Pathfinding* i_pathfinding) {
    players = i_players;
    playerCount = i_playerCount;
    terrain = i_terrain;
    worldRows = i_worldRows;
    worldCols = i_worldCols;
    pFinding = i_pathfinding;
}
