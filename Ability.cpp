#include "Ability.hpp"
#include <chrono>
using namespace std::chrono;

Ability::Ability(int i_myPlayerIndex, bool i_isFromNetwork) {
    finished = false;
    fromNetwork = i_isFromNetwork;
    myPlayerIndex = i_myPlayerIndex;
    addedToNetwork = false;
    phaseCount = 5;
    currentPhase = 0;

    phaseInitialized = new bool [phaseCount];
    phaseStart = new long long [phaseCount];
    phaseDuration = new int [phaseCount];
    timeBoundPhase = new bool [phaseCount];
    for (int i = 0; i < phaseCount; i++) {
        phaseInitialized [i] = false;
        timeBoundPhase [i] = false;
        phaseStart [i] = -1;
        phaseDuration [i] = -1;
    }
}

void Ability::update() {
    if (finished == false) {
        checkTime();
        executeCurrentPhase();
        if (currentPhase == 5) {
            finished = true;
        }
    }
}

void Ability::executeCurrentPhase() {
    if (currentPhase == 0) {
        execute0();
    }
    else if (currentPhase == 1) {
        execute1();
    }
    else if (currentPhase == 2) {
        execute2();
    }
    else if (currentPhase == 3) {
        execute3();
    }
    else if (currentPhase == 4) {
        execute4();
    }
}

void Ability::initCurrentPhase() {
    if (finished == false) {//only for big fuckups
        if (currentPhase == 1) {
            if (phaseInitialized [currentPhase] == false) {
                init1();
                phaseInitialized [currentPhase] = true;
            }
        }
        else if (currentPhase == 2) {
            if (phaseInitialized [currentPhase] == false) {
                init2();
                phaseInitialized [currentPhase] = true;
            }
        }
        if (currentPhase == 3) {
            if (phaseInitialized [currentPhase] == false) {
                init3();
                phaseInitialized [currentPhase] = true;
            }
        }
        else if (currentPhase == 4) {
            if (phaseInitialized [currentPhase] == false) {
                init4();
                phaseInitialized [currentPhase] = true;
            }
        }
    }
}

void Ability::checkTime() {
    if (timeBoundPhase [currentPhase] == true) {
        auto cTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        int diff = cTime - phaseStart [currentPhase];
        if (diff > phaseDuration [currentPhase]) {
            nextPhase();
        }
    }
}

void Ability::draw() {
    if (finished == false) {
        if (currentPhase == 0) {
            draw0();//needs no init, if this is not nullptr 0 is initialized caus constructor
        }
        if (currentPhase == 1) {
            if (phaseInitialized [currentPhase] == true) {
                draw1();
            }
        }
        if (currentPhase == 2) {
            if (phaseInitialized [currentPhase] == true) {
                draw2();
            }
        }
        if (currentPhase == 3) {
            if (phaseInitialized [currentPhase] == true) {
                draw3();
            }
        }
        if (currentPhase == 4) {
            if (phaseInitialized [currentPhase] == true) {
                draw4();
            }
        }
    }
}


//call this from the init-function of the to-be-time-limited phase
void Ability::endPhaseAfterMS(int ms) {
    timeBoundPhase [currentPhase] = true;
    phaseDuration [currentPhase] = ms;
    phaseStart [currentPhase] = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}