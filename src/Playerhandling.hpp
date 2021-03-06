#pragma once
#include <memory>
#include "PathfindingHandler.hpp"
#include "Player.hpp"


class PlayerHandling {
public:
	PlayerHandling();
	void draw();
	void sendPlayerData();

	/** Has to have pathfinding as a param so that we can update pathfinding-graph if player positions changed
	**/
	void receivePlayerData(int index);
	void createPlayer();
	inline void setPlayerIndex(int playerIndex) {
		myPlayerI = playerIndex;
	}

	inline int getPlayerIndex() {
		return myPlayerI;
	}

	inline shared_ptr<vector<shared_ptr<Player>>> getPlayers() {
		return players;
	}

	inline shared_ptr<Player> getMyPlayer() {
		return players->at(myPlayerI);
	}


private:
	shared_ptr<vector<shared_ptr<Player>>> players;//all players
	int myPlayerI;

};