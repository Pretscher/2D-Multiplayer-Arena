#pragma once
#include <math.h>


/*This is a base class for every ability. It has all the commonalities abilities need to be bound to this project
to use you have to override methods. Every ability should be cut into different phases, every phase MUST have
an initialization-, an update- and a draw-function. You can set if a phase should end after a certain amount
of time or you just manually end the phase with "nextPhase()". From outside, you just have to call update and draw
periodically to make this work. If you wish to add an ability through the network, you always have to consider the
differences in constructors and action-sequences. To help with that, in order to init an ability, you have to 
pass a bool that saves wether the ability was added through the network. You can get that bool through "isFormNetwork()".
The ability goes absolutely idle if "finished" is set to true. Then you only have to delete it. 
*/
class Ability {
public:

    /*This is a base class for every ability. It has all the commonalities abilities need to be bound to this project
    to use you have to override methods. Every ability should be cut into different phases, every phase MUST have
    an initialization-, an update- and a draw-function. You can set if a phase should end after a certain amount
    of time or you just manually end the phase with "nextPhase()". From outside, you just have to call update and draw
    periodically to make this work. If you wish to add an ability through the network, you always have to consider the
    differences in constructors and action-sequences. To help with that, in order to init an ability, you have to
    pass a bool that saves wether the ability was added through the network. You can get that bool through "isFormNetwork()".
    The ability goes absolutely idle if "finished" is set to true. Then you only have to delete it.
    */
    Ability(int i_myPlayerIndex, bool i_isFromNetwork);

    void update();
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

    void checkTime();
    void executeCurrentPhase();
    void initCurrentPhase();

    int currentPhase;
    int phaseCount;
    bool addedToNetwork;
};



