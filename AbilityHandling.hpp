#pragma once
#include "iostream" 
using namespace std;
#include "Player.hpp"
#include "Renderer.hpp"
#include "Terrain.hpp"
#include "Ability.hpp"
#include <vector>
#include <chrono>
#include <memory>
#include "VladW.hpp"
#include "VladE.hpp"
#include "VladR.hpp"

#include "Transfusion.hpp"
#include "Fireball.hpp"
#include "GlobalRecources.hpp"//init is called here
using namespace chrono;

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
        this->myPlayerI = i_myPlayerIndex;
        
        abilityTriggering = unique_ptr<AbilityTriggering>(new AbilityTriggering(abilityCount));

        //ability declerations
        declareCustomAbilities();

        newAbilities = vector<shared_ptr<Ability>>(abilityCount);
        for (int i = 0; i < abilityCount; i++) {
            newAbilities[i] = nullptr;
            hasNewAbility[i] = false;
        }
    }

    

	void update() {
        abilityTriggering->update();

        for (int i = 0; i < generalAbilities.size(); i++) {
            generalAbilities.at(i)->update();
        }

        for (int i = 0; i < abilityCount; i++) {
            if (hasNewAbility[i] == false) {//dont double init
                if (hasNewAbility[vladWindex] == false) {//cant cast in w
                    if (abilityTriggering->startAbility(i) == true) {//cooldown is ok and button is pressed
                        initAbility(i);
                    }
                }
            }
        }

        for (int i = 0; i < abilityCount; i++) {
            const Ability* cAbility = newAbilities[i].get();
            if (cAbility != nullptr) {

                //start cooldown only after target has been selected
                if (cAbility->wasPhaseInitialized(cAbility->getCDstartPhase()) == true) {
                    if (cAbility->isFromNetwork() == false) {
                        abilityTriggering->manuallyStartCooldown(cAbility->getAbilityIndex());
                    }
                }


                //delete if not needed anymore
                if (cAbility->finishedCompletely() == true) {
                    for (int j = 0; j < generalAbilities.size(); j++) {
                        if (generalAbilities.at(j) == newAbilities[i]) {
                            generalAbilities.erase(generalAbilities.begin() + j);
                        }

                    }
                    hasNewAbility[cAbility->getAbilityIndex()] = false;

                    newAbilities[i] = nullptr;
                    abilityTriggering->resetabilityStart(i);
                }
            }
        }
	}

	void drawAbilities() {
        for (int i = 0; i < generalAbilities.size(); i++) {
            generalAbilities.at(i)->draw();
        }
	}

    void drawCDs() {//a lot of hardcoded stuff because of imported ui texture. 
        int x = 1150;
        int size = 75;
        for (int i = 0; i < abilityCount; i++) {
            if (i > 0) {
                x += size + 10;
            }
            int y = 990;

            Renderer::drawRect(x, y, size, size, sf::Color(30, 30, 30, 255), true);
            int abilityRectHeight = 0;

            float cdPercent = abilityTriggering->getCooldownPercent(i);
            if (cdPercent > 0.01f && cdPercent < 0.99f) {
                abilityRectHeight = size * cdPercent;
            }
            else {
                cdPercent = 0;
            }
            Renderer::drawRect(x, y + size - (cdPercent * size), size, (cdPercent * size), sf::Color(0, 0, 150, 100), true);

            string abilityLetter;
            if (i == 0) {
                abilityLetter = string("Q");
            }
            else if (i == 1) {
                abilityLetter = string("W");
            }
            else if (i == 2) {
                abilityLetter = string("E");
            }
            else if (i == 3) {
                abilityLetter = string("R");
            }
            else if (i == 4) {
                abilityLetter = string("T");
            }
            Renderer::drawText(abilityLetter, x - size / 2.2f, y - size / 2.2f, size * 2.0f, size * 2.0f, 15, sf::Color(0, 0, 0, 255));
        }
    }


    void sendData() {
        for (int i = 0; i < abilityCount; i++) {
            if (hasNewAbility[i] == true) {
                if (newAbilities[i]->wasPhaseInitialized(newAbilities[i]->getAddToNetworkPhase()) && newAbilities[i]->wasAddedToNetwork() == false) {
                    hasNewAbility[i] = false;
                    newAbilities[i]->sendAbility();
                }
                else {
                    NetworkCommunication::addToken(0);
                }
            }
            else {
                NetworkCommunication::addToken(0);
            }
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
            generalAbilities.push_back(shared_ptr<Transfusion>(new Transfusion()));
        }

        if (NetworkCommunication::receiveNextToken() == 1) {
            generalAbilities.push_back(shared_ptr<VladW>(new VladW()));
        }

        if (NetworkCommunication::receiveNextToken() == 1) {
            generalAbilities.push_back(shared_ptr<VladE>(new VladE()));
        }

        if (NetworkCommunication::receiveNextToken() == 1) {
            generalAbilities.push_back(shared_ptr<VladR>(new VladR()));
        }
        if (NetworkCommunication::receiveNextToken() == 1) {
           generalAbilities.push_back(shared_ptr<Fireball>(new Fireball()));
        }

    }








    void initAbility(int abIndex) {
        hasNewAbility[abIndex] = true;
        //add abilities with respective indices
        switch (abIndex) {
        case 0: {
            //change this

            shared_ptr<Transfusion> newA(new Transfusion(myPlayerI));
            generalAbilities.push_back(newA);//has to be a pointer so that we can preserve it for newAbilities
            newAbilities[abIndex] = newA;
            break;
        }
        case 1: {
            shared_ptr<VladW> newA(new VladW(myPlayerI));
            generalAbilities.push_back(newA);
            newAbilities[abIndex] = newA;
            break;
        }
        case 2: {
            shared_ptr<VladE> newA(new VladE(myPlayerI));
            generalAbilities.push_back(newA);
            newAbilities[abIndex] = newA;
            break;
        }
        case 3: {
            shared_ptr<VladR> newA(new VladR(myPlayerI));
            generalAbilities.push_back(newA);
            newAbilities[abIndex] = newA;
            break;
        }
        case 4: {
            shared_ptr<Fireball> newA(new Fireball(myPlayerI));
            generalAbilities.push_back(newA);
            newAbilities[abIndex] = newA;
            break;

        }
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
    vector<shared_ptr<Ability>> generalAbilities;//gets deleted by shared-ptrs from Abilities
    vector<shared_ptr<Ability>> newAbilities;

    unique_ptr<AbilityTriggering> abilityTriggering;

    //NEW ABILITY ATTRIBUTES--------------------------------
    
    int abilityCount = 5;
    bool* hasNewAbility = new bool[abilityCount];

    //needed to push the abilities through the network with their individual values. TODO: Networking func for abilities

    //indices of abilities. Please set in "declareCustomAbilities()".
    int fireballIndex;
    int transfusionIndex;
    int vladEindex;
    int vladWindex;
    int vladRindex;
};
