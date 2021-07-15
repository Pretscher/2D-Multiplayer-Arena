#pragma once
#include <iostream>

class GraphNode {
public:
    bool isUseable;
    //initialized if set, only loopable with neighbourcount
    GraphNode** neighbours;
    float* neighbourCost;
    bool checkedByAlgorithm;
    GraphNode() {
        this->neighbours = new GraphNode *[8];
        this->neighbourCost = new float[8];
        this->isUseable = true;
        this->checkedByAlgorithm = false;
        this->heapIndex = -1;
        this->neighbourCount = 0;
        this->graphIndex = -1;
        this->myX = 0;
        this->myY = 0;
    }
    void linkNeighbour(GraphNode* neighbour) {
        this->setNeighbour(neighbour);
        neighbour->setNeighbour(this);
    }

    void setNeighbour(GraphNode* neighbour) {
        this->neighbours[neighbourCount] = neighbour;
        //default cost value to 1
        this->neighbourCost[neighbourCount] = 1;
        this->neighbourCount++;
    }

    void setNeighbourCost(unsigned int index, float heuristics) {
        this->neighbourCost[index] = heuristics;
    }

    float getNeighbourCost(unsigned int index) {
        return this->neighbourCost[index];
    }

    GraphNode* getNeighbour(int index) {
        return this->neighbours[index];
    }

    void setGraphIndex(unsigned int indexInGraphArray) {
        this->graphIndex = indexInGraphArray;
    }

    void setHeapIndex(unsigned int indexInHeapStructure) {
        this->heapIndex = indexInHeapStructure;
    }

    unsigned int getIndexInGraph() {
        return this->graphIndex;
    }

    unsigned int getHeapIndex() {
        return this->heapIndex;
    }

    unsigned int getNeighbourCount() {
        return this->neighbourCount;
    }

    int getX() {
        return this->myX;
    }
    int getY() {
        return this->myY;
    }
    void setX(int x) {
        this->myX = x;
    }
    void setY(int y) {
        this->myY = y;
    }

private:
    int myX, myY;
    unsigned int neighbourCount;
    unsigned int heapIndex;
    unsigned int graphIndex;
};