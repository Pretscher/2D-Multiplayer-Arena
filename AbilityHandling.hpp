#pragma once
#include "Player.hpp"
#include "Renderer.hpp"
#include "Terrain.hpp"
#include "Abilities.hpp"
#include <vector>
#include <chrono>
using namespace std::chrono;

class AbilityTriggering {
public:
    AbilityTriggering(int i_abilityCount) {
        this->abilityCount = i_abilityCount;//too lazy for doing this in a vector tbh
        abilities = new sf::Keyboard::Key[abilityCount];
        maxCDs = new int[abilityCount];

        cooldownStarts = new milliseconds[abilityCount];
        timeSinceCDStarts = new milliseconds[abilityCount];;

        samePress = new bool[abilityCount];
        onCD = new bool[abilityCount];
        createNewAbility = new bool[abilityCount];
        for(int i = 0; i < abilityCount; i++) {
            onCD[i] = false;
            samePress[i] = false;
            createNewAbility[i] = false;
        }
    }

    float getCooldownPercent(int index) {
        return ((float)maxCDs[index] - (float)timeSinceCDStarts[index].count()) / (float)maxCDs[index];
    }

    void addAbility(int index, sf::Keyboard::Key key, int cooldown) {
        abilities[index] = key;
        maxCDs[index] = cooldown;
    }

    void resetCooldown(int index){
        onCD[index] = false;
    }

    void manuallyStartCooldown(int index){
        onCD[index] = true;
        cooldownStarts[index] = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    }


    bool startAbility(int index) {
        bool temp = createNewAbility[index];
        //set to false after abilityHandler catches that a new abilty should be created
        createNewAbility[index] = false;
        return temp;
    }

    void update(){
        for(int i = 0; i < abilityCount; i++){
            if (onCD[i] == false) {
                if (sf::Keyboard::isKeyPressed(abilities[i]) == false) {
                    samePress[i] = false;
                }
                if (sf::Keyboard::isKeyPressed(abilities[i]) == true && samePress[i] == false) {
                    samePress[i] = true;
                    createNewAbility[i] = true;

                    cooldownStarts[i] = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
                    onCD[i] = true;
                }
            }
            else {
                timeSinceCDStarts[i] = duration_cast<milliseconds>(
                    system_clock::now().time_since_epoch()) - cooldownStarts[i];
                if (timeSinceCDStarts[i].count() >= maxCDs[i]) {
                    onCD[i] = false;
                }
            }
        }
    }

private:
    int abilityCount = 2;
    sf::Keyboard::Key* abilities;
    int* maxCDs;
    bool* onCD;
    milliseconds* cooldownStarts;
    milliseconds* timeSinceCDStarts;

    bool* samePress;

    bool* createNewAbility;
};

class AbilityHandling {
public:
    AbilityHandling(Player** i_players, int i_playerCount, Terrain* i_terrain, int i_worldRows, int i_worldCols, int i_myPlayerIndex, Pathfinding* i_pathfinding) {
        abilityRecources::init(i_players, i_playerCount, i_terrain, i_worldRows, i_worldCols, i_pathfinding);
        fireballs = new std::vector<Fireball*>();
        transfusions = new std::vector<Transfusion*>();
        this->myPlayerI = i_myPlayerIndex;
        
        abilityCount = 2;
        abilityTriggering = new AbilityTriggering(abilityCount);

        //ability declerations
        fireballIndex = 0;
        abilityTriggering->addAbility(fireballIndex, sf::Keyboard::Key::Q, 5000);//right now Fireball
        transfusionIndex = 1;
        abilityTriggering->addAbility(transfusionIndex, sf::Keyboard::Key::W, 2000);//right now Transfusion
    }

    

	void update() {
        abilityTriggering->update();
        
        if(abilityTriggering->startAbility(fireballIndex) == true){
            hasNewFireball = true;
            newFireball = new Fireball(myPlayerI);
            fireballs->push_back(newFireball);
        }
        if(abilityTriggering->startAbility(transfusionIndex) == true){
            newTransfusion = new Transfusion(myPlayerI);
            transfusions->push_back(newTransfusion);
            //start cooldown later when target has been selected
            abilityTriggering->resetCooldown(transfusionIndex);
        }

        for (int i = 0; i < fireballs->size(); i++) {
            fireballs->at(i)->update();
            if (fireballs->at(i)->finished == true) {
                fireballs->erase(fireballs->begin() + i);
            }
        }

        for (int i = 0; i < transfusions->size(); i++) {
            Transfusion* c = transfusions->at(i);
            c->update();
            if (c->finishedWithoutCasting == true || c->finishedCompletely == true) {
                transfusions->erase(transfusions->begin() + i);
            }
            //start cooldown only after target has been selected
            if(c->finishedSelectingTarget){
                abilityTriggering->manuallyStartCooldown(transfusionIndex);
            }
            if(c->casting == true){
                hasNewTransfusion = true;
                newTransfusion = c;
            }
        }
	}
	
	void drawAll() {
        for (int i = 0; i < fireballs->size(); i++) {
            fireballs->at(i)->draw();
        }
        for (int i = 0; i < transfusions->size(); i++) {
            transfusions->at(i)->draw();
        }

        int col = 0;
        for(int i = 0; i < abilityCount; i++) {
            col += 100;
            Renderer::drawRect(960, col, 100, 100, sf::Color(255, 100, 0, 255), true);
            int abilityRectHeight = 0;

            float cdPercent = abilityTriggering->getCooldownPercent(i);
            if (cdPercent > 0.01f) {
                abilityRectHeight = 100 * cdPercent;
            }
            Renderer::drawRect(960, col, 100, abilityRectHeight, sf::Color(0, 0, 255, 100), true);
        }
	}



    void sendData() {
        if (hasNewFireball == true) {
            hasNewFireball = false;
            NetworkCommunication::addToken(1);//check if new fireball is to be added
            NetworkCommunication::addToken(newFireball->startRow);
            NetworkCommunication::addToken(newFireball->startCol);
            NetworkCommunication::addToken(newFireball->goalRow);
            NetworkCommunication::addToken(newFireball->goalCol);
            NetworkCommunication::addToken(newFireball->myPlayerI);

        }
        else {
            NetworkCommunication::addToken(0);
        }

        if (hasNewTransfusion == true) {
            hasNewTransfusion = false;
            NetworkCommunication::addToken(1);//check if new transfusion is to be added
            NetworkCommunication::addToken(newTransfusion->myPlayerIndex);
            NetworkCommunication::addToken(newTransfusion->targetPlayerIndex);
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

            int startRow = NetworkCommunication::receiveNextToken();
            int startCol = NetworkCommunication::receiveNextToken();
            int goalRow = NetworkCommunication::receiveNextToken();
            int goalCol = NetworkCommunication::receiveNextToken();
            int firingPlayerIndex = NetworkCommunication::receiveNextToken();
            fireballs->push_back(new Fireball(startRow, startCol, goalRow, goalCol, firingPlayerIndex));
        }

        if (NetworkCommunication::receiveNextToken() == 1) {
            //theyre already in the right order

            int tMyPlayerIndex = NetworkCommunication::receiveNextToken();
            int tTargetPlayerIndex = NetworkCommunication::receiveNextToken();
            transfusions->push_back(new Transfusion(tMyPlayerIndex, tTargetPlayerIndex));
        }
    }

private:
    bool samePress = false;
    int myPlayerI;

    std::vector<Fireball*>* fireballs;
    std::vector<Transfusion*>* transfusions;
    Transfusion* newTransfusion;
    Fireball* newFireball;

    bool hasNewFireball = false;
    bool hasNewTransfusion = false;

    AbilityTriggering* abilityTriggering;

    int fireballIndex;
    int transfusionIndex;

    int abilityCount;
};
