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
    static int worldWidth, worldHeight;
    static int frameWidth, frameHeight;
    static shared_ptr<Terrain> terrain;
    static  shared_ptr<vector<shared_ptr<Player>>> players;
    static int playerCount;
    static Pathfinding* pFinding;
    static shared_ptr<mutex> pfMtx;
};
