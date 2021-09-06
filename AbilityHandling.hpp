#pragma once
#include "Player.hpp"
#include "Renderer.hpp"
#include "Terrain.hpp"
#include "Ability.hpp"
#include <vector>
#include <chrono>

#include "VladE.hpp"
#include "Transfusion.hpp"
#include "Fireball.hpp"
#include "GlobalRecources.hpp"//init is called here
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
        if (onCD [index] == false) {
            onCD [index] = true;
            cooldownStarts [index] = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        }
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
    AbilityHandling(int i_myPlayerIndex) {
        generalAbilities = new std::vector<Ability*>();
        this->myPlayerI = i_myPlayerIndex;
        
        abilityTriggering = new AbilityTriggering(abilityCount);

        //ability declerations
        declareCustomAbilities();

        newAbilities = new Ability * [abilityCount];
        for (int i = 0; i < abilityCount; i++) {
            newAbilities [i] = nullptr;
        }
    }

    

	void update() {
        abilityTriggering->update();

        for (int i = 0; i < generalAbilities->size(); i++) {
            generalAbilities->at(i)->update();
        }

        initAll();

        for (int i = 0; i < abilityCount; i++) {
            Ability* cAbility = newAbilities [i];
            if (cAbility != nullptr) {

                if (cAbility->wasPhaseInitialized(cAbility->getAddToNetworkPhase()) == true && cAbility->wasAddedToNetwork() == false) {
                    if (cAbility->isFromNetwork() == false) {
                        cAbility->addToNetwork();
                        hasNewAbility [cAbility->getAbilityIndex()] = true;
                        abilityTriggering->manuallyStartCooldown(cAbility->getAbilityIndex());//start cd at the same time you add to network
                    }
                }

                //start cooldown only after target has been selected
                if (cAbility->wasPhaseInitialized(cAbility->getCDstartPhase()) == true) {
                    if (cAbility->isFromNetwork() == false) {
                        abilityTriggering->manuallyStartCooldown(cAbility->getAbilityIndex());
                    }
                }


                //delete if not needed anymore
                if (cAbility->finishedCompletely() == true) {
                    for (int j = 0; j < generalAbilities->size(); j++) {
                        if (generalAbilities->at(j) == cAbility) {
                            generalAbilities->erase(generalAbilities->begin() + j);
                        }

                    }

                    deleteAll(cAbility);

                    delete newAbilities [i];
                    newAbilities [i] = nullptr;
                    hasNewAbility [fireballIndex] = false;
                }
            }
        }

        //add to network
        for (int i = 0; i < generalAbilities->size(); i++) {
            Ability* c = generalAbilities->at(i);
            if (c->wasAddedToNetwork() == false) {
                if (c->wasPhaseInitialized(c->getAddToNetworkPhase()) == true && c->getCastingPlayer() == myPlayerI 
                                                                            && c->wasAddedToNetwork() == false) {
                    c->addToNetwork();
                    hasNewAbility [c->getAbilityIndex()] = true;
                }
            }
        }
	}

	void drawAbilities() {
        for (int i = 0; i < generalAbilities->size(); i++) {
            generalAbilities->at(i)->draw();
        }
	}

    void drawCDs() {//a lot of hardcoded stuff because of imported ui texture. 
        int col = 1150;
        int size = 75;
        for (int i = 0; i < abilityCount; i++) {
            if (i > 0) {
                col += size + 10;
            }
            int row = 990;

            Renderer::drawRect(row, col, size, size, sf::Color(30, 30, 30, 255), true);
            int abilityRectHeight = 0;

            float cdPercent = abilityTriggering->getCooldownPercent(i);
            if (cdPercent > 0.01f && cdPercent < 0.99f) {
                abilityRectHeight = size * cdPercent;
            }
            else {
                cdPercent = 0;
            }
            Renderer::drawRect(row + size - (cdPercent * size), col, size, (cdPercent * size), sf::Color(0, 0, 150, 100), true);

            std::string abilityLetter;
            if (i == 0) {
                abilityLetter = std::string("Q");
            }
            else if (i == 1) {
                abilityLetter = std::string("W");
            }
            else {
                abilityLetter = std::string("E");
            }
            Renderer::drawText(abilityLetter, row - size / 2.2f, col - size / 2.2f, size * 2.0f, size * 2.0f, sf::Color(0, 0, 0, 255));
        }
    }


    void sendData() {
        if (hasNewAbility[fireballIndex] == true) {
            hasNewAbility [fireballIndex] = false;
            NetworkCommunication::addToken(1);//check if new fireball is to be added

            Fireball* newFireball = nullptr;
            for (int i = 0; i < fireballs->size(); i++) {
                if (fireballs->at(i) == newAbilities [fireballIndex]) {
                    newFireball = fireballs->at(i);
                    break;
                }
            }

            NetworkCommunication::addToken(newFireball->getProjectileRow());
            NetworkCommunication::addToken(newFireball->getProjectileCol());
            NetworkCommunication::addToken(newFireball->getGoalRow());
            NetworkCommunication::addToken(newFireball->getGoalCol());
            NetworkCommunication::addToken(newFireball->getCastingPlayer());
            NetworkCommunication::addToken(newFireball->getPhase());
            NetworkCommunication::addToken(newFireball->getStartTime(2));
        }
        else {
            NetworkCommunication::addToken(0);
        }

        if (hasNewAbility [transfusionIndex] == true) {
            hasNewAbility [transfusionIndex] = false;

            Transfusion* newTransfusion = nullptr;
            for (int i = 0; i < transfusions->size(); i++) {
                if (transfusions->at(i) == newAbilities [transfusionIndex]) {
                    newTransfusion = transfusions->at(i);
                    break;
                }
            }
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
            int phase = NetworkCommunication::receiveNextToken();
            int timeSinceExplosionStart = NetworkCommunication::receiveNextToken();

            Fireball* newFB = new Fireball(startRow, startCol, goalRow, goalCol, firingPlayerIndex, phase, timeSinceExplosionStart);
            fireballs->push_back(newFB);
            generalAbilities->push_back(newFB);
        }

        if (NetworkCommunication::receiveNextToken() == 1) {
            //theyre already in the right order

            int tMyPlayerIndex = NetworkCommunication::receiveNextToken();
            int tTargetPlayerIndex = NetworkCommunication::receiveNextToken();

            Transfusion* newT = new Transfusion(tMyPlayerIndex, tTargetPlayerIndex);

            transfusions->push_back(newT);
            generalAbilities->push_back(newT);
        }
    }








    void initAll() {
        initFireball();
        initTransfusion();
        initVladE();
    }

    void deleteAll(Ability* cAbility) {
        deleteFireball(cAbility);
        deleteTransfusion(cAbility);
        deleteVladE(cAbility);
    }

    void initFireball() {
        if (newAbilities [fireballIndex] == nullptr) {
            if (abilityTriggering->startAbility(fireballIndex) == true) {
                Fireball* newFB = new Fireball(myPlayerI);

                fireballs->push_back(newFB);
                generalAbilities->push_back(newFB);
                newAbilities [fireballIndex] = newFB;
            }
        }
    }

    void initTransfusion() {
        if (newAbilities [transfusionIndex] == nullptr) {
            if (abilityTriggering->startAbility(transfusionIndex) == true) {
                Transfusion* newT = new Transfusion(myPlayerI);

                transfusions->push_back(newT);
                generalAbilities->push_back(newT);
                newAbilities [transfusionIndex] = newT;
                //start cooldown later when target has been selected
            }
        }
    }

    void initVladE() {
        if (newAbilities [vladEindex] == nullptr) {
            if (abilityTriggering->startAbility(vladEindex) == true) {
                VladE* newE = new VladE(myPlayerI);

                vladEs->push_back(newE);
                generalAbilities->push_back(newE);
                newAbilities [vladEindex] = newE;
                //start cooldown later when target has been selected
            }
        }
    }

    void deleteVladE(Ability* toDelete) {
        for (int c = 0; c < vladEs->size(); c++) {
            if (vladEs->at(c) == toDelete) {
                vladEs->erase(vladEs->begin() + c);
            }
        }
    }

    void deleteFireball(Ability* toDelete) {
        for (int c = 0; c < fireballs->size(); c++) {
            if (fireballs->at(c) == toDelete) {
                fireballs->erase(fireballs->begin() + c);
            }
        }
    }

    void deleteTransfusion(Ability* toDelete) {
        for (int c = 0; c < transfusions->size(); c++) {
            if (transfusions->at(c) == toDelete) {
                transfusions->erase(transfusions->begin() + c);
            }
        }
    }

    void declareCustomAbilities() {
        fireballIndex = 0;
        abilityTriggering->addAbility(fireballIndex, sf::Keyboard::Key::Q, 5000);//right now Fireball
        transfusionIndex = 1;
        abilityTriggering->addAbility(transfusionIndex, sf::Keyboard::Key::W, 2000);//right now Transfusion
        vladEindex = 2;
        abilityTriggering->addAbility(vladEindex, sf::Keyboard::Key::E, 2000);//right now Transfusion
    }

private:
    bool samePress = false;
    int myPlayerI;
    std::vector<Ability*>* generalAbilities;
    Ability** newAbilities;

    AbilityTriggering* abilityTriggering;

    //NEW ABILITY ATTRIBUTES--------------------------------
    
    int abilityCount = 3;
    bool* hasNewAbility = new bool [abilityCount];

    std::vector<Fireball*>* fireballs = new std::vector<Fireball*>();
    std::vector<Transfusion*>* transfusions = new std::vector<Transfusion*>();
    std::vector<VladE*>* vladEs = new std::vector<VladE*>();;

    //indices of abilities. Please set in "declareCustomAbilities()".
    int fireballIndex;
    int transfusionIndex;
    int vladEindex;
};
