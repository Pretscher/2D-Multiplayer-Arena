#include "Graph.hpp"
#include "Renderer.hpp"
#include <iostream>
#include "Algorithm.hpp"


Graph::Graph(int i_rows, int i_cols, float i_accuracy) {
    accuracy = i_accuracy;
    delete[] this->currentGraph;
    this->graphNodeCount = 0;

    this->cols = i_cols * accuracy;
    this->rows = i_rows * accuracy;
    int lenght = this->rows * cols;

    //init everything with given sizes
    neighbourCount = new int[lenght];
    neighbourIndices = new int* [lenght];//for every node there can be 8 neighbours, second dimension initialized in loop
    xCoords = new int[lenght];
    yCoords = new int[lenght];
    currentGraph = new int[lenght];
    heapIndices = new int[lenght];
    usedByMoveable = new bool[lenght];

    rawIndices = new int* [this->rows];
    neighbourCosts = nullptr; //initialized later for every run of algorithm


    blockedX = new std::vector<int>();
    blockedY = new std::vector<int>();
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
    delete[] usedByMoveable;
}

void Graph::reset() {
    for (int i = 0; i < graphNodeCount; i++) {
        delete[] neighbourCosts[i];
    }
    delete[] heapIndices;
    heapIndices = new int[this->rows * this->cols];
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
                usedByMoveable[graphNodeCount] = false;
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
            }
        } //end one row
    } //end all rows
}

int Graph::findNextUseableVertex(int row, int col, bool moveableRelevant) {
    //do (basically) breathfirstsearch: add 1 to rows, then 1 to cols. then add minus 1 to rows and cols. then +2 -2 etc. Till you 
    //found the next best useable node, which is also the nearest. 

    int addRow = 0;//increment for rows and cols (swaps between negative and positive so it goes in every direction)
    int addCol = 0;
    int rowCounter = 0;//swap between adding and not adding in every iteration for rows
    int colCounter = 0;//swap between adding and not adding in every iteration for cols
    bool changeRow = true;//swap between rows and cols
    int tempRow = row;//initialize to row so the loop doesnt break instantly (we know rawIndices[row][col] is not accessible)
    int tempCol = col;

    int cIndex = -1;
    //while no suitable index found (rawIndices[tempRow][tempCol] is -1 if unuseable) and in bounds of window)
    while (cIndex == -1) {
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
                if (row + addRow >= rows) {
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
                if (col + addCol >= cols) {
                    addCol = -addCol;//revert change to addRow so it only goes into one direction
                }
            }
            tempCol = col + addCol;
        }
        changeRow = !changeRow; //swap between rows and cols
        cIndex = rawIndices[tempRow][tempCol];

        if (moveableRelevant == true) {
            if (cIndex != -1) {
                if (usedByMoveable[cIndex] == true) {
                    cIndex = -1;
                }
            }
        }

        if (tempRow >= rows || tempCol >= cols) {
            std::cout << "\n\n\ncouldnt find vertex----------------------------\n\n\n";
            break;
        }
    }
    if (cIndex > 0) {
        return cIndex;
    }
    else {
        std::cout << "error in findUseableVertex";
        std::exit(0);
    }
}

int Graph::getIndexFromCoords(int row, int col, bool moveableRelevant) {
    row = row * accuracy;
    col = col * accuracy;

    if (row < rows && col < cols) {
        if (moveableRelevant == true) {
            if (rawIndices[row][col] >= 0 && usedByMoveable[rawIndices[row][col]] == false) {
                return rawIndices[row][col];
            }
            else {
                return findNextUseableVertex(row, col, moveableRelevant);
            }
        }
        else {
            if (rawIndices[row][col] >= 0) {
                return rawIndices[row][col];
            }
            else {
                return findNextUseableVertex(row, col, moveableRelevant);
            }
        }
    }
    else {
        std::cout << "out of bounds row or col in 'Graph->getIndexFromCoords'";
        std::exit(0);
    }
}

void Graph::disableObjectBounds(int row, int col, int width, int height) {
    row *= accuracy;
    col *= accuracy;
    width *= accuracy;
    height *= accuracy;

    int minY = row - height + 1;
    if (minY < 0) {
        minY = 0;
    }
    int minX = col - width + 1;
    if (minX < 0) {
        minX = 0;
    }
    for (int y = minY; y < row + height; y++) {
        for (int x = minX; x < col + width; x++) {
           // if (Renderer::currentWindow != nullptr) {
                //Renderer::drawRect(y / accuracy, x / accuracy, 2, 2, sf::Color(255, 255, 0, 255));
           // }
            int index = getIndexFromCoords(y / accuracy, x / accuracy, false);
            usedByMoveable[index] = true;       
        }
    }
}

void Graph::enableObjectBounds(int row, int col, int width, int height) {
    row *= accuracy;
    col *= accuracy;
    width *= accuracy;
    height *= accuracy;

    int minY = row - height + 1;
    if (minY < 0) {
        minY = 0;
    }
    int minX = col - width + 1;
    if (minX < 0) {
        minX = 0;
    }
    for (int y = minY; y < row + height; y++) {
        for (int x = minX; x < col + width; x++) {
            int index = getIndexFromCoords(y / accuracy, x / accuracy, false);
            usedByMoveable[index] = false;
        }
    }
}

void Graph::moveObject(int row, int col, int oldRow, int oldCol, int width, int height) {
    enableObjectBounds(oldRow, oldCol, width, height);
    disableObjectBounds(row, col, width, height);
}