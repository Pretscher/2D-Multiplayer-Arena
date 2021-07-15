#include "HeapNode.hpp"
#include "Algorithm.hpp"

float HeapNode::getKey() {
    return myKey;
}

unsigned int HeapNode::getIndexInGraph() {
    if (usedInIteration <= Algorithm::currentIteration && usedInIteration != -1) {//-1 means never used
        myKey = std::numeric_limits<float>::max();
        myIndexInGraph = -1;
        usedInIteration = Algorithm::currentIteration;
    }

    return myIndexInGraph;
}

void HeapNode::setIndex(unsigned int index) {
    myIndexInGraph = index;
}

void HeapNode::setKey(float key) {
    myKey = key;
}