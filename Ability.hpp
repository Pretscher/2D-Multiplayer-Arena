#pragma once
#include <math.h>

class Ability {
public:
    Ability(int i_myPlayerIndex, bool i_isFromNetwork);

    void update();
    
    void executeCurrentPhase();
    void initCurrentPhase();
    void checkTime();
    void draw();
    //call this from the init-function of the to-be-time-limited phase
    void endPhaseAfterMS(int ms);
    inline void skipToPhase(int phase) {
        currentPhase = phase;
        initCurrentPhase();
    }
    inline void nextPhase() {
        currentPhase ++;
        initCurrentPhase();
    }


    //getters/setters
    inline int getPhase() {
        return currentPhase;
    }

    inline bool finishedPhase(int index) {
        return index < currentPhase;//if current Phase is higher than index, phase with index was finished
    }
    
    inline bool wasPhaseInitialized(int index) {
        return phaseInitialized [index];
    }

    inline bool finishedCompletely() {
        return finished;
    }

    inline bool wasAddedToNetwork() {
        return addedToNetwork;
    }

    inline void addToNetwork() {
        addedToNetwork = true;
    }

    inline long long getStartTime(int index) {
        return phaseStart [index];
    }

    inline int getCastingPlayer() {
        return myPlayerIndex;
    }

    inline bool isFromNetwork() {
        return fromNetwork;
    }

protected:
    bool finished;
    int myPlayerIndex;

    bool* phaseInitialized;

    bool* timeBoundPhase;
    long long* phaseStart;
    int* phaseDuration;

    bool fromNetwork;

    virtual void init1() { finished = true; }
    virtual void init2() { finished = true; }
    virtual void init3() { finished = true; }
    virtual void init4() { finished = true; }

    virtual void execute0() { finished = true; }
    virtual void execute1() { finished = true; }
    virtual void execute2() { finished = true; }
    virtual void execute3() { finished = true; }
    virtual void execute4() { finished = true; }

    virtual void draw0() { finished = true; }
    virtual void draw1() { finished = true; }
    virtual void draw2() { finished = true; }
    virtual void draw3() { finished = true; }
    virtual void draw4() { finished = true; }

private:
    int currentPhase;
    int phaseCount;
    bool addedToNetwork;
};



