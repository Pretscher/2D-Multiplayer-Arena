#include "Graph.hpp"
#include "Renderer.hpp"
#include <iostream>
#include "Algorithm.hpp"


Graph::Graph(int i_rows, int i_cols) {
    delete[] this->currentGraph;
    this->graphNodeCount = 0;

    this->cols = i_cols;
    this->rows = i_rows;
    int lenght = this->rows * this->cols;

    //init everything with given sizes
    neighbourCount = new int[lenght];
    neighbourIndices = new int* [lenght];//for every node there can be 8 neighbours, second dimension initialized in loop
    xCoords = new int[lenght];
    yCoords = new int[lenght];
    currentGraph = new int[lenght];
    heapIndices = new int[lenght];
    isUseable = new bool* [this->rows]; //double array so we init for y and x (in loop) seperately

    rawIndices = new int* [this->rows];
    neighbourCosts = nullptr; //initialized later for every run of algorithm
}

/* MEMO:
isUseable = new bool* [this->rows];
for (int y = 0; y < this->rows; y++) {
    isUseable[y] = new bool[this->cols];
}*/

void Graph::generateWorldGraph(bool** isUseable) {
    for (int y = 0; y < rows; y++) {
        //one row
        rawIndices[y] = new int[cols];

        for (int x = 0; x < cols; x++) {
            neighbourIndices[graphNodeCount] = new int[8];
            //all rows and cols have been created

            //this should be initialized in "makeRectNodesUnusable()" if it was declared unusable
            //we have to do one less iteration through everything if we ask that here :)
            if (isUseable[y][x] != false) {
                isUseable[y][x] = true;
            }
            if (isUseable[y][x] == true) {
                rawIndices[y][x] = graphNodeCount;

                xCoords[graphNodeCount] = x;
                yCoords[graphNodeCount] = y;

                //isUseable is false if it has been initialized as such in "makeRectNodesUnusable()"
                neighbourCount[graphNodeCount] = 0;
                int xI;
                int yI;
                if (x > 0) {
                    xI = x - 1;
                    yI = y;
                    if (isUseable[yI][xI] == true) {
                        neighbourIndices[rawIndices[yI][xI]][neighbourCount[rawIndices[yI][xI]]] = graphNodeCount;
                        neighbourCount[rawIndices[yI][xI]]++;
                        neighbourIndices[graphNodeCount][neighbourCount[graphNodeCount]] = rawIndices[yI][xI];
                        neighbourCount[graphNodeCount]++;
                    }
                }
                if (y > 0) {
                    xI = x;
                    yI = y - 1;
                    if (isUseable[yI][xI] == true) {
                        neighbourIndices[rawIndices[yI][xI]][neighbourCount[rawIndices[yI][xI]]] = graphNodeCount;
                        neighbourCount[rawIndices[yI][xI]]++;
                        neighbourIndices[graphNodeCount][neighbourCount[graphNodeCount]] = rawIndices[yI][xI];
                        neighbourCount[graphNodeCount]++;
                    }
                }
                if (y > 0 && x > 0) {
                    xI = x - 1;
                    yI = y - 1;
                    if (isUseable[yI][xI] == true) {
                        neighbourIndices[rawIndices[yI][xI]][neighbourCount[rawIndices[yI][xI]]] = graphNodeCount;
                        neighbourCount[rawIndices[yI][xI]]++;
                        neighbourIndices[graphNodeCount][neighbourCount[graphNodeCount]] = rawIndices[yI][xI];
                        neighbourCount[graphNodeCount]++;
                    }
                }
                if (y > 0 && x < cols - 1) {
                    xI = x + 1;
                    yI = y - 1;
                    if (isUseable[yI][xI] == true) {
                        neighbourIndices[rawIndices[yI][xI]][neighbourCount[rawIndices[yI][xI]]] = graphNodeCount;
                        neighbourCount[rawIndices[yI][xI]]++;
                        neighbourIndices[graphNodeCount][neighbourCount[graphNodeCount]] = rawIndices[yI][xI];
                        neighbourCount[graphNodeCount]++;
                    }
                }
                currentGraph[graphNodeCount] = graphNodeCount;
                graphNodeCount++;
            } //End one node
        } //end one row
    } //end all rows
}
