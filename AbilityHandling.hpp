#pragma once
#include "Player.hpp"
#include "Renderer.hpp"
#include "Terrain.hpp"
#include "Abilities.hpp"
#include <vector>
#include <chrono>
class AbilityHandling {
public:
    AbilityHandling(Player** i_players, int i_playerCount, Terrain* i_terrain, int i_worldRows, int i_worldCols, int i_myPlayerIndex) {
        abilityRecources::init(i_players, i_playerCount, i_terrain, i_worldRows, i_worldCols);
        fireballs = new std::vector<Fireball*>();
        this->myPlayerI = i_myPlayerIndex;
    }

    
    clock_t cooldownStart;
	void update() {
        if (qOnCooldown == false) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q) == false) {
                samePress = false;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q) == true && samePress == false) {
                samePress = true;

                hasNewFireball = true;
                newFireball = new Fireball(myPlayerI);
                fireballs->push_back(newFireball);
                cooldownStart = clock();
                qOnCooldown = true;
            }
        }
        else {
            timeSinceCdStart = clock() - cooldownStart;
            if (timeSinceCdStart >= qCooldown) {
                qOnCooldown = false;
            }
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

    float getQCooldownPercentLeft() {
        return ((float)qCooldown - (float)timeSinceCdStart) / (float)qCooldown;
    }



    void sendData() {
        if (hasNewFireball == true) {
            NetworkCommunication::addToken(1);//check if new fireball is to be added
            NetworkCommunication::addToken(newFireball->startRow);
            NetworkCommunication::addToken(newFireball->startCol);
            NetworkCommunication::addToken(newFireball->goalRow);
            NetworkCommunication::addToken(newFireball->goalCol);
            NetworkCommunication::addToken(newFireball->myPlayerI);
            hasNewFireball = false;
        }
        else {
            NetworkCommunication::addToken(0);
        }
    }

    /** Has to pass pathfinding so that we can update pathfinding-graph if player positions changed
    **/
    void receiveData() {
        int otherPlayer = 0;
        if (myPlayerI == 0) {
            otherPlayer = 1;
        }
        if (NetworkCommunication::receiveNextToken() == 1) {
            //theyre already in the right order
            fireballs->push_back(new Fireball(NetworkCommunication::receiveNextToken(),
                NetworkCommunication::receiveNextToken(), NetworkCommunication::receiveNextToken(), 
                NetworkCommunication::receiveNextToken(), NetworkCommunication::receiveNextToken()));
        }
    }

private:
    bool samePress = false;
    int myPlayerI;

    int qCooldown = 5000;
    bool qOnCooldown = false; 
    clock_t timeSinceCdStart;
    std::vector<Fireball*>* fireballs;
    Fireball* newFireball;
    bool hasNewFireball = false;
};
