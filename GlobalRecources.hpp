#pragma once
#include "Player.hpp"
#include "Terrain.hpp"
#include "PathfindingHandler.hpp"
#include "iostream" 
using namespace std;

/*A static class for all abilities to use, those are just generally pretty useful attributes but they are all in use, it's an
"add it if you need it" approach.*/ 
class GlobalRecources {
public:
    static void init(Player** i_players, int i_playerCount, Terrain* i_terrain, int i_worldHeight, int i_worldWidth, Pathfinding* i_pathfinding, mutex* i_pfMtx);

    static int worldWidth, worldHeight;
    static int frameWidth, frameHeight;
    static Terrain* terrain;
    static Player** players;
    static int playerCount;
    static Pathfinding* pFinding;
    static mutex* pfMtx;
};
