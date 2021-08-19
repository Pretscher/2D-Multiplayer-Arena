#pragma once
#include "Player.hpp"
#include "Renderer.hpp"
#include "Terrain.hpp"
#include "Abilities.hpp"
#include <vector>
class AbilityHandling {
public:
    AbilityHandling(Player** i_players, int i_playerCount, Terrain* i_terrain, int i_worldRows, int i_worldCols, int i_myPlayerIndex) {
        abilityRecources::init(i_players, i_playerCount, i_terrain, i_worldRows, i_worldCols);
        fireballs = new std::vector<Fireball*>();
        this->myPlayerI = i_myPlayerIndex;
    }

    

	void update() {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q) == false) {
            samePress = false;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q) == true && samePress == false) {
            samePress = true;

            fireballs->push_back(new Fireball(myPlayerI));
        }

        for (int i = 0; i < fireballs->size(); i++) {
            fireballs->at(i)->update();
            if (fireballs->at(i)->dead == true) {
                fireballs->erase(fireballs->begin() + i);
            }
        }
	}
	
	void drawAll() {
        for (int i = 0; i < fireballs->size(); i++) {
            fireballs->at(i)->draw();
        }
	}

private:
    bool samePress = false;
    int myPlayerI;

    std::vector<Fireball*>* fireballs;
};
