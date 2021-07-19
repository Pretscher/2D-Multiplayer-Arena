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

    rawIndices = new int* [this->rows];
    neighbourCosts = nullptr; //initialized later for every run of algorithm
}

Graph::~Graph() {
    delete[] xCoords;
    delete[] yCoords;
    delete[] neighbourCount;
    delete[] heapIndices;
    delete[] currentGraph;
    for (int i = 0; i < graphNodeCount; i++) {
        delete[] neighbourIndices[i];
        delete[] neighbourCosts[i];
        if (i < rows) {
            delete[] rawIndices[i];
        }
    }
    delete[] neighbourCosts;
    delete[] neighbourIndices;
    delete[] rawIndices;
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
            else {
                rawIndices[y][x] = -1;
                xCoords[graphNodeCount] = -1;
                yCoords[graphNodeCount] = -1;
            }
        } //end one row
    } //end all rows
}

bool changeRow;
int rowCounter;
int colCounter;
int findNextUseableVertex(int** rawIndices, int maxRow, int maxCol, int row, int col) {
    //do (basically) breathfirstsearch: add 1 to rows, then 1 to cols. then add minus 1 to rows and cols. then +2 -2 etc. Till you 
    //found the next best useable node, which is also the nearest. 

    int addRow = 0;//increment for rows and cols (swaps between negative and positive so it goes in every direction)
    int addCol = 0;
    rowCounter = 0;//swap between adding and not adding in every iteration for rows
    colCounter = 0;//swap between adding and not adding in every iteration for cols
    changeRow = true;//swap between rows and cols
    int tempRow = row;//initialize to row so the loop doesnt break instantly (we know rawIndices[row][col] is not accessible)
    int tempCol = col;

    //while no suitable index found (rawIndices[tempRow][tempCol] is -1 if unuseable) and in bounds of window)
    while (rawIndices[tempRow][tempCol] == -1) {
        if (changeRow == true) {
            if (rowCounter == 1) {//only add every second iteration in rows so that its not (+1 -2 +3 -4 etc.) but (+1 -1 +2 -2 etc.)
                addRow ++;//increment row indention until some node might be found
                rowCounter = 0;
            }
            else {
                rowCounter = 1;
            }
            addRow = -addRow;//+1 -1 +2 -2 etc
            if (addRow < 0){
                if (row + addRow < 0) {
                    addRow = -addRow;//revert change to addRow so it only goes into one direction
                }
            }
            else {
                if (row + addRow >= maxRow) {
                    addRow = -addRow;//revert change to addRow so it only goes into one direction
                }
            }
            tempRow = row + addRow;//add calculated row-indention
        }
        if (changeRow == false) {//same as for row
            if (rowCounter == 1) {
                addCol ++;
                colCounter = 0;
            }
            else {
                colCounter = 1;
            }

            addCol = -addCol;
            if (addCol < 0) {
                if (col + addCol < 0) {
                    addCol = -addCol;//revert change to addRow so it only goes into one direction
                }
            }
            else {
                if (col + addCol >= maxCol) {
                    addCol = -addCol;//revert change to addRow so it only goes into one direction
                }
            }
            tempCol = col + addCol;
        }
        changeRow = !changeRow; //swap between rows and cols
    }
    if (rawIndices[tempRow][tempCol] > 0) {
        return rawIndices[tempRow][tempCol];
    }
    else {
        std::cout << "error in findUseableVertex";
        std::exit(0);
    }
}

int Graph::getIndexFromCoords(int row, int col) {
    if (row < rows && col < cols) {
        if (rawIndices[row][col] >= 0) {
            return rawIndices[row][col];
        }
        else {
            return findNextUseableVertex(rawIndices, rows, cols, row, col);
        }
    }
    else {
        std::cout << "out of bounds row or col in 'Graph->getIndexFromCoords'";
        std::exit(0);
    }
    
    
}