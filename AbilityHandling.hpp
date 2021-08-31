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

            cooldownStarts [i] = milliseconds(0);
            timeSinceCDStarts [i] = milliseconds(0);
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
        
        if (fireballIndicatorActive == false) {
            if (abilityTriggering->startAbility(fireballIndex) == true) {
                newFireball = new Fireball(myPlayerI);
                fireballs->push_back(newFireball);
                fireballIndicatorActive = true;
            }
        }
        else {
            //If fireball finished selecting destination, transmit through network asap
            if (newFireball->finishedPhase(0) == true && newFireball->wasAddedToNetwork() == false) {
                newFireball->addToNetwork();
                hasNewFireball = true;
                abilityTriggering->manuallyStartCooldown(fireballIndex);
            }
        }

        //transfusion cooldown handling
        if (transfusionIndicatorActive == false) {
            if (abilityTriggering->startAbility(transfusionIndex) == true) {
                transfusionIndicatorActive = true;
                newTransfusion = new Transfusion(myPlayerI);
                transfusions->push_back(newTransfusion);
                //start cooldown later when target has been selected
            }
        }

        for (int i = 0; i < fireballs->size(); i++) {
            fireballs->at(i)->update();
            if (fireballs->at(i)->finishedCompletely() == true || fireballs->at(i)->hasFinishedNoCast() == true) {
                fireballs->erase(fireballs->begin() + i);
                
                if (newFireball != nullptr) {
                    delete newFireball;
                    newFireball = nullptr;
                    hasNewFireball = false;
                }

                fireballIndicatorActive = false;
            }
        }

        for (int i = 0; i < transfusions->size(); i++) {
            Transfusion* c = transfusions->at(i);
            c->update();
            if (c->hasEndedNoCast() == true || c->hasFinishedCast() == true) {
                transfusions->erase(transfusions->begin() + i);
                //dont pass through network anymore
                if (newTransfusion != nullptr) {
                    delete newTransfusion;
                    newTransfusion = nullptr;
                    hasNewTransfusion = false;
                }
                transfusionIndicatorActive = false;
            }
            if (c != nullptr) {
                //start cooldown only after target has been selected
                if (c->hasSelectedTarget() == true) {
                    if (c->getTargetPlayer() != -1) {//TODO can we delete this? dunno anymore
                        abilityTriggering->manuallyStartCooldown(transfusionIndex);
                    }
                    transfusionIndicatorActive = false;
                }
                if (c->isCasting() == true && c->getCastingPlayer() == myPlayerI && c->wasAddedToNetwork() == false) {
                    c->setAddedToNetwork();
                    hasNewTransfusion = true;
                    newTransfusion = c;
                }
            }
        }
	}

	void drawAbilities() {
        for (int i = 0; i < fireballs->size(); i++) {
            fireballs->at(i)->draw();
        }
        for (int i = 0; i < transfusions->size(); i++) {
            transfusions->at(i)->draw();
        }
	}

    void drawCDs() {
        int col = 0;
        for (int i = 0; i < abilityCount; i++) {
            col += 150;
            Renderer::drawRect(960, col, 100, 100, sf::Color(255, 100, 0, 255), true);
            int abilityRectHeight = 0;

            float cdPercent = abilityTriggering->getCooldownPercent(i);
            if (cdPercent > 0.01f && cdPercent < 0.99f) {
                abilityRectHeight = 100 * cdPercent;
            }
            Renderer::drawRect(960, col, 100, abilityRectHeight, sf::Color(0, 0, 255, 100), true);

            std::string abilityLetter;
            if (i == 0) {
                abilityLetter = std::string("Q");
            }
            if (i == 1) {
                abilityLetter = std::string("W");
            }
            Renderer::drawText(abilityLetter, 910, col - 45, 200, 200, sf::Color(0, 0, 0, 255));
        }
    }


    void sendData() {
        if (hasNewFireball == true) {
            hasNewFireball = false;
            NetworkCommunication::addToken(1);//check if new fireball is to be added
            NetworkCommunication::addToken(newFireball->getProjectileRow());
            NetworkCommunication::addToken(newFireball->getProjectileCol());
            NetworkCommunication::addToken(newFireball->getGoalRow());
            NetworkCommunication::addToken(newFireball->getGoalCol());
            NetworkCommunication::addToken(newFireball->getCastingPlayer());
            NetworkCommunication::addToken(newFireball->getPhase());
            auto a = newFireball->getStartTime(2);
            NetworkCommunication::addToken(newFireball->getStartTime(2));

        }
        else {
            NetworkCommunication::addToken(0);
        }

        if (hasNewTransfusion == true) {
            hasNewTransfusion = false;
            NetworkCommunication::addToken(1);//check if new transfusion is to be added
            NetworkCommunication::addToken(newTransfusion->getCastingPlayer());
            NetworkCommunication::addToken(newTransfusion->getTargetPlayer());
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
            int exploding = NetworkCommunication::receiveNextToken();
            int timeSinceExplosionStart = NetworkCommunication::receiveNextToken();
            fireballs->push_back(new Fireball(startRow, startCol, goalRow, goalCol, firingPlayerIndex, exploding, timeSinceExplosionStart));
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
    Transfusion* newTransfusion = nullptr;
    Fireball* newFireball = nullptr;

    bool hasNewFireball = false;
    bool hasNewTransfusion = false;

    AbilityTriggering* abilityTriggering;

    int fireballIndex;
    int transfusionIndex;

    int abilityCount;
    bool transfusionIndicatorActive = false;
    bool fireballIndicatorActive = false;
};
