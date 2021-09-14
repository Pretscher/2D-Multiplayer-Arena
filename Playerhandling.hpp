#pragma once

#include "PathfindingHandler.hpp"
#include "Player.hpp"


class PlayerHandling {
public:
	PlayerHandling();
	void draw();
	void sendPlayerData();

	/** Has to have pathfinding as a param so that we can update pathfinding-graph if player positions changed
	**/
	void receivePlayerData(Pathfinding&& pathfinding);

	inline void setPlayerIndex(int playerIndex) {
		myPlayerI = playerIndex;
	}

	inline int getPlayerIndex() {
		return myPlayerI;
	}

	inline Player** getPlayers() {
		return players;
	}

	inline Player* getMyPlayer() {
		return players[myPlayerI];
	}

	inline int getPlayerCount() {
		return playerCount;
	}



private:
	Player** players;//all players
	int myPlayerI;
	int playerCount;//number of players

};