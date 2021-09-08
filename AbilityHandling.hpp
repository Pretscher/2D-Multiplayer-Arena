#pragma once
#include "Player.hpp"
#include "Renderer.hpp"
#include "Terrain.hpp"
#include "Ability.hpp"
#include <vector>
#include <chrono>

#include "VladW.hpp"
#include "VladE.hpp"
#include "VladR.hpp"

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

            cooldownStarts[i] = milliseconds(0);
            timeSinceCDStarts[i] = milliseconds(0);
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
        if (onCD[index] == false) {
            onCD[index] = true;
            cooldownStarts[index] = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        }
    }


    bool startAbility(int index) {
        bool temp = createNewAbility[index];
        //set to false after abilityHandler catches that a new abilty should be created
        createNewAbility[index] = false;
        return temp;
    }

    void resetabilityStart(int index) {
        createNewAbility[index] = false;
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

        newAbilities = new Ability *[abilityCount];
        for (int i = 0; i < abilityCount; i++) {
            newAbilities[i] = nullptr;
        }
    }

    

	void update() {
        abilityTriggering->update();

        for (int i = 0; i < generalAbilities->size(); i++) {
            generalAbilities->at(i)->update();
        }

        for (int i = 0; i < abilityCount; i++) {
            if (newAbilities[i] == nullptr) {//dont double init
                if (abilityTriggering->startAbility(i) == true) {//cooldown is ok and button is pressed
                    initAbility(i);
                }
            }
        }

        for (int i = 0; i < abilityCount; i++) {
            Ability* cAbility = newAbilities[i];
            if (cAbility != nullptr) {

                if (cAbility->wasPhaseInitialized(cAbility->getAddToNetworkPhase()) == true && cAbility->wasAddedToNetwork() == false) {
                    if (cAbility->isFromNetwork() == false) {
                        cAbility->addToNetwork();
                        hasNewAbility[cAbility->getAbilityIndex()] = true;
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
                    hasNewAbility[cAbility->getAbilityIndex()] = false;
                    deleteAbility(cAbility);

                    delete cAbility;
                    newAbilities[i] = nullptr;
                    abilityTriggering->resetabilityStart(i);
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
                    hasNewAbility[c->getAbilityIndex()] = true;
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
            else if (i == 2) {
                abilityLetter = std::string("E");
            }
            else if (i == 3) {
                abilityLetter = std::string("R");
            }
            else if (i == 4) {
                abilityLetter = std::string("T");
            }
            Renderer::drawText(abilityLetter, row - size / 2.2f, col - size / 2.2f, size * 2.0f, size * 2.0f, sf::Color(0, 0, 0, 255));
        }
    }


    void sendData() {
        if (hasNewAbility[fireballIndex] == true) {
            hasNewAbility[fireballIndex] = false;
            NetworkCommunication::addToken(1);//check if new fireball is to be added

            Fireball* newFireball = nullptr;
            for (int i = 0; i < fireballs->size(); i++) {
                if (fireballs->at(i) == newAbilities[fireballIndex]) {
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

        if (hasNewAbility[transfusionIndex] == true) {
            hasNewAbility[transfusionIndex] = false;

            Transfusion* newTransfusion = nullptr;
            for (int i = 0; i < transfusions->size(); i++) {
                if (transfusions->at(i) == newAbilities[transfusionIndex]) {
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

        if (hasNewAbility[vladEindex] == true) {
            hasNewAbility[vladEindex] = false;

            VladE* newE = nullptr;
            for (int i = 0; i < vladEs->size(); i++) {
                if (vladEs->at(i) == newAbilities[vladEindex]) {
                    newE = vladEs->at(i);
                    break;
                }
            }
            NetworkCommunication::addToken(1);//check if new transfusion is to be added
            NetworkCommunication::addToken(newE->getCastingPlayer());
            NetworkCommunication::addToken(newE->getPhase());

            auto cTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            int timeSinceStart = cTime - newE->getStartTime(newE->getPhase());
            NetworkCommunication::addToken(timeSinceStart);
        }
        else {
            NetworkCommunication::addToken(0);
        }

        if (hasNewAbility[vladWindex] == true) {
            hasNewAbility[vladWindex] = false;

            VladW* newW = nullptr;
            for (int i = 0; i < vladWs->size(); i++) {
                if (vladWs->at(i) == newAbilities[vladWindex]) {
                    newW = vladWs->at(i);
                    break;
                }
            }
            NetworkCommunication::addToken(1);//check if new transfusion is to be added
            NetworkCommunication::addToken(newW->getCastingPlayer());

            auto cTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            int timeSinceStart = cTime - newW->getStartTime(newW->getPhase());
            NetworkCommunication::addToken(timeSinceStart);
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

            Fireball* c = new Fireball(startRow, startCol, goalRow, goalCol, firingPlayerIndex, phase, timeSinceExplosionStart);
            fireballs->push_back(c);
            generalAbilities->push_back(c);
        }

        if (NetworkCommunication::receiveNextToken() == 1) {
            //theyre already in the right order

            int tMyPlayerIndex = NetworkCommunication::receiveNextToken();
            int tTargetPlayerIndex = NetworkCommunication::receiveNextToken();

            Transfusion* c = new Transfusion(tMyPlayerIndex, tTargetPlayerIndex);

            transfusions->push_back(c);
            generalAbilities->push_back(c);
        }

        if (NetworkCommunication::receiveNextToken() == 1) {
            //theyre already in the right order

            int tMyPlayerIndex = NetworkCommunication::receiveNextToken();
            int phase = NetworkCommunication::receiveNextToken();
            int timeSincePhaseStart = NetworkCommunication::receiveNextToken();

            VladE* c = new VladE(tMyPlayerIndex, phase, timeSincePhaseStart);

            vladEs->push_back(c);
            generalAbilities->push_back(c);
        }

        if (NetworkCommunication::receiveNextToken() == 1) {
            //theyre already in the right order

            int tMyPlayerIndex = NetworkCommunication::receiveNextToken();
            int timeSincePhaseStart = NetworkCommunication::receiveNextToken();

            VladW* c = new VladW(tMyPlayerIndex, timeSincePhaseStart);

            vladWs->push_back(c);
            generalAbilities->push_back(c);
        }

    }








    void initAbility(int abIndex) {

        //add abilities with respective indices
        switch (abIndex) {
        case 0: {
            //change this
            Transfusion* newT = new Transfusion(myPlayerI);
            transfusions->push_back(newT);
            generalAbilities->push_back(newT);
            newAbilities[abIndex] = newT;
            break;
        }
        case 1: {
            //change this
            VladW* newW = new VladW(myPlayerI);
            vladWs->push_back(newW);
            generalAbilities->push_back(newW);
            newAbilities[abIndex] = newW;
            break;
        }
        case 2: {
            //change this
            VladE* newE = new VladE(myPlayerI);
            vladEs->push_back(newE);
            generalAbilities->push_back(newE);
            newAbilities[abIndex] = newE;
            break;
        }
        case 3: {
            //change this
            VladR* newR = new VladR(myPlayerI);
            vladRs->push_back(newR);
            generalAbilities->push_back(newR);
            newAbilities[abIndex] = newR;
            break;
        }
        case 4: {
            //change this
            Fireball* newFB = new Fireball(myPlayerI);
            fireballs->push_back(newFB);
            generalAbilities->push_back(newFB);
            newAbilities[abIndex] = newFB;
            break;

        }
        default:
            break;
        }
    }
    

    void deleteAbility(Ability* toDelete) {
        //add abilities with respective indices
        switch (toDelete->getAbilityIndex()) {
        case 0:
            for (int c = 0; c < transfusions->size(); c++) {
                if (transfusions->at(c) == toDelete) {
                    transfusions->erase(transfusions->begin() + c);
                }
            }
            break;
        case 1:
            for (int c = 0; c < vladWs->size(); c++) {
                if (vladWs->at(c) == toDelete) {
                    vladWs->erase(vladWs->begin() + c);
                }
            }
            break;
        case 2:
            for (int c = 0; c < vladEs->size(); c++) {
                if (vladEs->at(c) == toDelete) {
                    vladEs->erase(vladEs->begin() + c);
                }
            }
            break;
        case 3:

            for (int c = 0; c < vladRs->size(); c++) {
                if (vladRs->at(c) == toDelete) {
                    vladRs->erase(vladRs->begin() + c);
                }
            }
            break;
        case 4:

            for (int c = 0; c < fireballs->size(); c++) {
                if (fireballs->at(c) == toDelete) {
                    fireballs->erase(fireballs->begin() + c);
                }
            }
            break;
        default:
            break;
        }
    }

    void declareCustomAbilities() {

        transfusionIndex = 0;
        abilityTriggering->addAbility(transfusionIndex, sf::Keyboard::Key::Q, 2000);
        vladWindex = 1;
        abilityTriggering->addAbility(vladWindex, sf::Keyboard::Key::W, 5000);
        vladEindex = 2;
        abilityTriggering->addAbility(vladEindex, sf::Keyboard::Key::E, 2000);
        vladRindex = 3;
        abilityTriggering->addAbility(vladRindex, sf::Keyboard::Key::R, 5000);

        fireballIndex = 4;
        abilityTriggering->addAbility(fireballIndex, sf::Keyboard::Key::T, 5000);
    }

private:
    bool samePress = false;
    int myPlayerI;
    std::vector<Ability*>* generalAbilities;
    Ability** newAbilities;

    AbilityTriggering* abilityTriggering;

    //NEW ABILITY ATTRIBUTES--------------------------------
    
    int abilityCount = 5;
    bool* hasNewAbility = new bool[abilityCount];

    std::vector<Fireball*>* fireballs = new std::vector<Fireball*>();
    std::vector<Transfusion*>* transfusions = new std::vector<Transfusion*>();

    std::vector<VladE*>* vladEs = new std::vector<VladE*>();
    std::vector<VladW*>* vladWs = new std::vector<VladW*>();
    std::vector<VladR*>* vladRs = new std::vector<VladR*>();

    //indices of abilities. Please set in "declareCustomAbilities()".
    int fireballIndex;
    int transfusionIndex;
    int vladEindex;
    int vladWindex;
    int vladRindex;
};
