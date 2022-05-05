#pragma once
#include <iostream>
using namespace std;
#include <limits>

class HeapNode {
public:
    HeapNode(float key, int indexInGraph) {
        myKey = key;
        myIndexInGraph = indexInGraph;
    }

    inline float getKey() {
        return myKey;
    }

    inline unsigned int getIndexInGraph(int currentIteration) {
        if (usedInIteration <= currentIteration && usedInIteration != -1) {//-1 means never used
            myKey = numeric_limits<float>::max();
            myIndexInGraph = -1;
            usedInIteration = currentIteration;
        }

        return myIndexInGraph;
    }

    inline void getIndex(unsigned int index) {
        myIndexInGraph = index;
    }

    inline void setKey(float key) {
        myKey = key;
    }

    inline int getIteration() {
        return usedInIteration;
    }
private:
    int usedInIteration = -1;
    float myKey = numeric_limits<float>::max();
    int myIndexInGraph = -1;
};