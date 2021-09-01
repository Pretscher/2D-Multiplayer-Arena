#pragma once
#include "Player.hpp"
#include "Terrain.hpp"
#include "PathfindingHandler.hpp"

/*A static class for all abilities to use, those are just generally pretty useful attributes but they are all in use, it's an
"add it if you need it" approach.*/ 
class AbilityRecources {
public:
    static void init(Player** i_players, int i_playerCount, Terrain* i_terrain, int i_worldRows, int i_worldCols, Pathfinding* i_pathfinding);

    static int worldRows, worldCols;
    static Terrain* terrain;
    static Player** players;
    static int playerCount;
    static Pathfinding* pFinding;
};
