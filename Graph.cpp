#include "Graph.hpp"
#include "Renderer.hpp"
#include <iostream>
#include "Algorithm.hpp"


Graph::Graph(int i_rows, int i_cols, float i_accuracy) {
    accuracy = i_accuracy;
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
    delete[] neighbourCosts;
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

struct IntPoint{
    int row;
    int col;
    IntPoint(int i_row, int i_col) {
        row = i_row;
        col = i_col;
    }
};

int Graph::findNextUseableVertex(int row, int col, bool moveableRelevant) {
    //do (basically) breathfirstsearch: add 1 to rows, then 1 to cols. then add minus 1 to rows and cols. then +2 -2 etc. Till you 
    //found the next best useable node, which is also the nearest. 

    int cIndex = -1;

    bool** visited = new bool*[rows];
    for (int y = 0; y < rows; y++) {
        visited[y] = new bool[cols];
        for (int x = 0; x < cols; x++) {
            visited[y][x] = false;
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
            if (cRow + 1 < rows) {
                if (visited[cRow + 1][cCol] == false) {
                    toVisit->push_back(IntPoint(cRow + 1, cCol));
                    visited[cRow + 1][cCol] = true;//dont use node again
                }
            }
            if (cCol + 1 < cols) {
                if (visited[cRow][cCol + 1] == false) {
                    toVisit->push_back(IntPoint(cRow, cCol + 1));
                    visited[cRow][cCol + 1] = true;//dont use node again
                }
            }
            if (cRow - 1 >= 0) {
                if (visited[cRow - 1][cCol] == false) {
                    toVisit->push_back(IntPoint(cRow - 1, cCol));
                    visited[cRow - 1][cCol] = true;//dont use node again
                }
            }
            if (cCol - 1 >= 0) {
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

    for (int y = 0; y < rows; y++) {
        delete[] visited[y];
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

bool debug = false;

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
    for (int y = minY; y <= row + height; y++) {
        for (int x = minX; x <= col + width; x++) {
          // if (Renderer::currentWindow != nullptr) {
             //  Renderer::drawRect(y / accuracy, x / accuracy, 2, 2, sf::Color(255, 255, 0, 255));
         //  }
            int index = getIndexFromCoords(y / accuracy, x / accuracy, false);
            usedByMoveable[index] = true;
            if (debug == true) {
                deactivatedX.push_back(x / accuracy);
                deactivatedY.push_back(y / accuracy);
            }
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
    for (int y = minY; y <= row + height; y++) {
        for (int x = minX; x <= col + width; x++) {
            int index = getIndexFromCoords(y / accuracy, x / accuracy, false);
            usedByMoveable[index] = false;

            if (debug == true) {
                for (int i = 0; i < deactivatedY.size(); i++) {
                    if (deactivatedY.at(i) == y / accuracy && deactivatedX.at(i) == x / accuracy) {
                        deactivatedX.erase(deactivatedX.begin() + i);
                        deactivatedY.erase(deactivatedY.begin() + i);
                    }
                }
            }
        }
    }
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