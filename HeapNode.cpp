#include "HeapNode.hpp"
#include "Algorithm.hpp"
#include "iostream" 
using namespace std;
float HeapNode::getKey() {
    return myKey;
}

unsigned int HeapNode::getIndexInGraph(int currentIteration) {
    if (usedInIteration <= currentIteration && usedInIteration != -1) {//-1 means never used
        myKey = numeric_limits<float>::max();
        myIndexInGraph = -1;
        usedInIteration = currentIteration;
    }

    return myIndexInGraph;
}

void HeapNode::setIndex(unsigned int index) {
    myIndexInGraph = index;
}

void HeapNode::setKey(float key) {
    myKey = key;
}