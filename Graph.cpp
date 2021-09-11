#include "Graph.hpp"
#include "Renderer.hpp"
#include <iostream>
#include "Algorithm.hpp"
#include <math.h>
#include "GlobalRecources.hpp"

Graph::Graph(int i_rows, int i_cols, float i_accuracy) {
    accuracy = i_accuracy;
    this->graphNodeCount = 0;
    this->colCount = (float)i_cols * accuracy;
    this->rowCount = (float)i_rows * accuracy;
    int lenght = this->rowCount * colCount;

    //init everything with given sizes
    neighbourCount = new int[lenght];
    neighbourIndices = new int*[lenght];//for every node there can be 8 neighbours, second dimension initialized in loop
    indexBoundCols = new int[lenght];
    indexBoundRows = new int[lenght];
    currentGraph = new int[lenght];
    heapIndices = new int[lenght];
    usedByMoveable = new bool[lenght];

    rawIndices = new int*[this->rowCount];
    neighbourCosts = nullptr; //initialized later for every run of algorithm

    deactivatedX = std::vector<int>();
    deactivatedY = std::vector<int>();
}

Graph::~Graph() {
    delete[] indexBoundCols;
    delete[] indexBoundRows;
    delete[] neighbourCount;
    delete[] heapIndices;
    delete[] currentGraph;
    for (int i = 0; i < graphNodeCount; i++) {
        delete[] neighbourIndices[i];
        delete[] neighbourCosts[i];
        if (i < rowCount) {
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
    delete[] neighbourCosts;
    delete[] heapIndices;
    heapIndices = new int[this->rowCount * this->colCount];
}

/* MEMO:
isUseable = new bool*[this->rowCount];
for (int row = 0; row < this->rowCount; row++) {
    isUseable[row] = new bool[this->colCount];
}*/

void Graph::generateWorldGraph(bool** isUseable) {
    for (int row = 0; row < rowCount; row++) {
        //one row
        rawIndices[row] = new int[colCount];

        for (int col = 0; col < colCount; col++) {
            neighbourIndices[graphNodeCount] = new int[8];
            //all rowCount and colCount have been created

            //this should be initialized in "makeRectNodesUnusable()" if it was declared unusable
            //we have to do one less iteration through everything if we ask that here :)
            if (isUseable[row][col] == true) {
                rawIndices[row][col] = graphNodeCount;
                usedByMoveable[graphNodeCount] = false;
                indexBoundCols[graphNodeCount] = col;
                indexBoundRows[graphNodeCount] = row;

                //isUseable is false if it has been initialized as such in "makeRectNodesUnusable()"
                neighbourCount[graphNodeCount] = 0;
                int colIndex;
                int rowIndex;
                if (col > 0) {
                    colIndex = col - 1;
                    rowIndex = row;
                    if (isUseable[rowIndex][colIndex] == true) {
                        neighbourIndices[rawIndices[rowIndex][colIndex]][neighbourCount[rawIndices[rowIndex][colIndex]]] = graphNodeCount;
                        neighbourCount[rawIndices[rowIndex][colIndex]]++;
                        neighbourIndices[graphNodeCount][neighbourCount[graphNodeCount]] = rawIndices[rowIndex][colIndex];
                        neighbourCount[graphNodeCount]++;
                    }
                }
                if (row > 0) {
                    colIndex = col;
                    rowIndex = row - 1;
                    if (isUseable[rowIndex][colIndex] == true) {
                        neighbourIndices[rawIndices[rowIndex][colIndex]][neighbourCount[rawIndices[rowIndex][colIndex]]] = graphNodeCount;
                        neighbourCount[rawIndices[rowIndex][colIndex]]++;
                        neighbourIndices[graphNodeCount][neighbourCount[graphNodeCount]] = rawIndices[rowIndex][colIndex];
                        neighbourCount[graphNodeCount]++;
                    }
                }
                if (row > 0 && col > 0) {
                    colIndex = col - 1;
                    rowIndex = row - 1;
                    if (isUseable[rowIndex][colIndex] == true) {
                        neighbourIndices[rawIndices[rowIndex][colIndex]][neighbourCount[rawIndices[rowIndex][colIndex]]] = graphNodeCount;
                        neighbourCount[rawIndices[rowIndex][colIndex]]++;
                        neighbourIndices[graphNodeCount][neighbourCount[graphNodeCount]] = rawIndices[rowIndex][colIndex];
                        neighbourCount[graphNodeCount]++;
                    }
                }
                if (row > 0 && col < colCount - 1) {
                    colIndex = col + 1;
                    rowIndex = row - 1;
                    if (isUseable[rowIndex][colIndex] == true) {
                        neighbourIndices[rawIndices[rowIndex][colIndex]][neighbourCount[rawIndices[rowIndex][colIndex]]] = graphNodeCount;
                        neighbourCount[rawIndices[rowIndex][colIndex]]++;
                        neighbourIndices[graphNodeCount][neighbourCount[graphNodeCount]] = rawIndices[rowIndex][colIndex];
                        neighbourCount[graphNodeCount]++;
                    }
                }

                currentGraph[graphNodeCount] = graphNodeCount;
                graphNodeCount++;
            } //End one node
            else {
                rawIndices[row][col] = -1;
            }
        } //end one row
    } //end all rowCount
}

struct IntPoint{
    int row;
    int col;
    IntPoint(int i_row, int i_col) {
        row = i_row;
        col = i_col;
    }
};

int Graph::findNextUseableVertex(int row, int col, bool moveableRelevant) {
    //do (basically) breathfirstsearch: add 1 to rowCount, then 1 to colCount. then add minus 1 to rowCount and colCount. then +2 -2 etc. Till you 
    //found the next best useable node, which is also the nearest. 

    int cIndex = -1;

    bool** visited = new bool*[rowCount];
    for (int row = 0; row < rowCount; row++) {
        visited[row] = new bool[colCount];
        for (int col = 0; col < colCount; col++) {
            visited[row][col] = false;
        }
    }
    


    std::vector<IntPoint>* toVisit = new std::vector<IntPoint>();
    //while no suitable index found (rawIndices[tempRow][tempCol] is -1 if unuseable) and in bounds of window)
    IntPoint cPoint = IntPoint(row, col);
    toVisit->push_back(cPoint);
    while (cIndex == -1) {
        //those two commands are basically "dequeue" from a regular queue.
        cPoint = toVisit->at(0);
        toVisit->erase(toVisit->begin());
        int cRow = cPoint.row;
        int cCol = cPoint.col;


        visited[cRow][cCol] = true;//dont use node again
        for (int i = 0; i < 4; i++) {
            if (cRow + 1 < rowCount) {//bound check
                if (visited[cRow + 1][cCol] == false) {
                    toVisit->push_back(IntPoint(cRow + 1, cCol));
                    visited[cRow + 1][cCol] = true;//dont use node again
                }
            }
            if (cCol + 1 < colCount) {//bound check
                if (visited[cRow][cCol + 1] == false) {
                    toVisit->push_back(IntPoint(cRow, cCol + 1));
                    visited[cRow][cCol + 1] = true;//dont use node again
                }
            }
            if (cRow - 1 >= 0) {//bound check
                if (visited[cRow - 1][cCol] == false) {
                    toVisit->push_back(IntPoint(cRow - 1, cCol));
                    visited[cRow - 1][cCol] = true;//dont use node again
                }
            }
            if (cCol - 1 >= 0) {//bound check
                if (visited[cRow][cCol - 1] == false) {
                    toVisit->push_back(IntPoint(cRow, cCol - 1));
                    visited[cRow][cCol - 1] = true;//dont use node again
                }
            }
        }

       

        cIndex = rawIndices[cRow][cCol];//check if index is valid now
        if (cIndex != -1) {
            if (moveableRelevant == true) {
                if (usedByMoveable[cIndex] == true) {
                    cIndex = -1;
                }
            }
        }
    }

    for (int row = 0; row < rowCount; row++) {
        delete[] visited[row];
    }
    delete[] visited;
    delete toVisit;
    if (cIndex > 0) {
        return cIndex;
    }
    else {
        std::cout << "error in findUseableVertex";
        std::exit(0);
    }
}

void Graph::findNextUseableCoords(int* io_x, int* io_y, bool moveableRelevant) {
    float col = (float) *io_x * accuracy;
    float row = (float) *io_y * accuracy;

    int cIndex = rawIndices[(int)row][(int)col];//check if index is valid now
    if (cIndex != -1) {
        if (moveableRelevant == true) {
            if (usedByMoveable[cIndex] == true) {
                cIndex = -1;
            }
        }
    }
    if (cIndex != -1) {
        return;//dont change coords
    }

    int index = findNextUseableVertex(row, col, moveableRelevant);
    *io_x = (float)indexBoundCols[index] / accuracy;
    *io_y = (float)indexBoundRows[index] / accuracy;
}

int Graph::getIndexFromCoords(int row, int col, bool moveableRelevant) {
    row = round(((float)row * accuracy));
    col = round(((float)col * accuracy));

    if (row < rowCount && col < colCount) {
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



bool debug = false;

void Graph::disableObjectBounds(int row, int col, int width, int height) {
    GlobalRecources::pfMtx->lock();
    row = (float) row * accuracy;
    col = (float) col * accuracy;
    width = (float) width * accuracy;
    height = (float) height * accuracy;

    int minY = row - height + 1;
    if (minY < 0) {
        minY = 0;
    }
    int minX = col - width + 1;
    if (minX < 0) {
        minX = 0;
    }

    int maxX = col + width;
    if (maxX >= colCount) {
        maxX = colCount - 2;
    }
    int maxY = row + height;
    if (maxY >= rowCount) {
        maxY = rowCount - 2;
    }

    for (int row = minY; row <= maxY; row++) {
        for (int col = minX; col <= maxX; col++) {
          // if (Renderer::currentWindow != nullptr) {
             //  Renderer::drawRect(row / accuracy, col / accuracy, 2, 2, sf::Color(255, 255, 0, 255));
         //  }
            int index = getIndexFromCoords((float) row / accuracy, (float) col / accuracy, false);
            usedByMoveable[index] = true;
            if (debug == true) {
                deactivatedX.push_back((float) col / accuracy);
                deactivatedY.push_back((float) row / accuracy);
            }
        }
    }

    GlobalRecources::pfMtx->unlock();
}

void Graph::enableObjectBounds(int row, int col, int width, int height) {
    GlobalRecources::pfMtx->lock();

    row = (float) row * accuracy;
    col = (float) col * accuracy;
    width = (float) width * accuracy;
    height = (float) height * accuracy;

    int minY = row - height + 1;
    if (minY < 0) {
        minY = 0;
    }
    int minX = col - width + 1;
    if (minX < 0) {
        minX = 0;
    }   

    int maxX = col + width;
    if (maxX >= colCount) {
        maxX = colCount - 2;
    }
    int maxY = row + height;
    if (maxY >= rowCount) {
        maxY = rowCount - 2;
    }

    for (int row = minY; row <= maxY; row ++) {
        for (int col = minX; col <= maxX; col ++) {
            int index = getIndexFromCoords((float) row / accuracy, (float) col / accuracy, false);
            usedByMoveable[index] = false;

            if (debug == true) {
                for (int i = 0; i < deactivatedY.size(); i++) {
                    if (deactivatedY.at(i) == (float)row / accuracy && deactivatedX.at(i) == (float)col / accuracy) {
                        deactivatedX.erase(deactivatedX.begin() + i);
                        deactivatedY.erase(deactivatedY.begin() + i);
                    }
                }
            }
        }
    }

    GlobalRecources::pfMtx->unlock();
}

void Graph::moveObject(int row, int col, int oldRow, int oldCol, int width, int height) {
    enableObjectBounds(oldRow, oldCol, width, height);
    disableObjectBounds(row, col, width, height);
}



void Graph::debugDrawing() {
    debug = true;
    for (int i = 0; i < deactivatedY.size(); i++) {
        Renderer::drawRect(deactivatedY.at(i), deactivatedX.at(i), 2, 2, sf::Color(255, 255, 0, 255), false);
    }
}